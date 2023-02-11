/*
Florenc Caminade
Thomas FLayols
Etienne Arlaud

Receive raw 802.11 packet and filter ESP-NOW vendor specific action frame using BPF filters.
https://hackaday.io/project/161896
https://github.com/thomasfla/Linux-ESPNOW

Adapted from :
https://stackoverflow.com/questions/10824827/raw-sockets-communication-over-wifi-receiver-not-able-to-receive-packets

1/Find your wifi interface:
$ iwconfig

2/Setup your interface in monitor mode :
$ sudo ifconfig wlp5s0 down
$ sudo iwconfig wlp5s0 mode monitor
$ sudo ifconfig wlp5s0 up

3/Run this code as root
*/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <assert.h>
#include <linux/filter.h>
#include <string.h>
#include <time.h>

#include "espnow_packet.h"

#define DATA_LEN 4

#define MAX_PACKET_LEN 1000

typedef struct timespec stopwatch_t;

static inline void print_time(stopwatch_t timer)
{
    printf("%lds %ldns\n", timer.tv_sec, timer.tv_nsec);
}

stopwatch_t time_diff(stopwatch_t timer_end, stopwatch_t timer_start)
{
    stopwatch_t timer_res;
    timer_res.tv_sec = timer_end.tv_sec - timer_end.tv_sec;
    timer_res.tv_nsec = timer_end.tv_nsec - timer_start.tv_nsec;
    if (timer_res.tv_nsec < 0) 
    {
        timer_res.tv_sec -= 1;
        timer_res.tv_nsec += 1000 * 1000 * 1000;
    }
    return timer_res;
}

struct sound_entry {
    stopwatch_t timestamp;
    int32_t mcu_ind;
};

#define STORAGE_STEP 256 

struct storage {
    struct sound_entry *data;
    uint32_t cnt;
    uint32_t size;
}; 

void zero_storage(struct storage *s) {
    s->data = NULL;
    s->cnt = 0;
    s->size = 0;
}

// todo: change return type and error check
void add_entry(struct storage *s, struct sound_entry entry) {
    if (s->cnt == s->size) {
        uint32_t new_size = s->size + STORAGE_STEP;
        struct sound_entry *new_data = realloc(s->data, sizeof(struct sound_entry) * new_size);
        if (!new_data) {
            fprintf(stderr, "failed to allocate memory for new entry\n");
            return;
        }
        s->data = new_data;
        s->size = new_size;
    }
    s->data[s->cnt] = entry;
    s->cnt++;
}

void clean_storage(struct storage *s) {
    free(s->data);
    s->data = NULL;
    s->cnt = 0;
    s->size = 0;
}


#define MAX_MCUS 100
static uint8_t mac_mapping[MAX_MCUS][6];
static int32_t stored_macs = 0;


void process_packet(struct storage *s, uint8_t *data, int len) 
{
    stopwatch_t timestamp;
    // int clock_res; // TODO: add error checking
    clock_gettime(CLOCK_REALTIME, &timestamp);

    if (len < 0) {
        fprintf(stderr, "error reading from socket\n");
        return;
    }

    long espnow_packet_len = sizeof(espnow_packet_t);
    if (len != espnow_packet_len) {
        fprintf(stderr, "unexpected packet length\n");
        return;
    }

    espnow_packet_t *p = (espnow_packet_t*) data;

    uint8_t *source_mac = p->wlan.sa;
    int32_t mac_ind = -1;

    for (int32_t i = 0; i < stored_macs; i++) {
        if (memcmp(source_mac, mac_mapping[i], 6) == 0) {
            mac_ind = i;
            break;
        }
    }

    if (mac_ind == -1) {
        mac_ind = stored_macs;
        stored_macs++;
        memcpy(mac_mapping[mac_ind], source_mac, 6);
    }

    struct sound_entry entry;
    entry.timestamp = timestamp;
    entry.mcu_ind = mac_ind;

    add_entry(s, entry);

    printf("packet from %d\n", mac_ind);
    printf("received at:\n");
    print_time(timestamp);
}

/*our MAC address*/
// {4c:77:cb:1a:58:ce }

/*ESP8266 host MAC address*/
// {58:bf:25:db:62:a6} --outdated


//filter action frame packets
  //Equivalent for tcp dump :
    //type 0 subtype 0xd0 and wlan[24:4]=0x7f18fe34 and wlan[32]=221 and wlan[33:4]&0xffffff = 0x18fe34 and wlan[37]=0x4
//NB : There is no filter on source or destination addresses, so this code will 'receive' the action frames sent by this computer...
#define FILTER_LENGTH 20
static struct sock_filter bpfcode[FILTER_LENGTH] = {
  { 0x30, 0, 0, 0x00000003 },   // ldb [3]  // radiotap header length : MS byte
  { 0x64, 0, 0, 0x00000008 },   // lsh #8   // left shift it
  { 0x7, 0, 0, 0x00000000 },    // tax      // 'store' it in X register
  { 0x30, 0, 0, 0x00000002 },   // ldb [2]  // radiotap header length : LS byte
  { 0x4c, 0, 0, 0x00000000 },   // or  x    // combine A & X to get radiotap header length in A
  { 0x7, 0, 0, 0x00000000 },    // tax      // 'store' it in X
  { 0x50, 0, 0, 0x00000000 },   // ldb [x + 0]      // right after radiotap header is the type and subtype
  { 0x54, 0, 0, 0x000000fc },   // and #0xfc        // mask the interesting bits, a.k.a 0b1111 1100
  { 0x15, 0, 10, 0x000000d0 },  // jeq #0xd0 jt 9 jf 19 // compare the types (0) and subtypes (0xd)
  { 0x40, 0, 0, 0x00000018 },   // Ld  [x + 24]         // 24 bytes after radiotap header is the end of MAC header, so it is category and OUI (for action frame layer)
  { 0x15, 0, 8, 0x7f18fe34 },   // jeq #0x7f18fe34 jt 11 jf 19  // Compare with category = 127 (Vendor specific) and OUI 18:fe:34
  { 0x50, 0, 0, 0x00000020 },   // ldb [x + 32]             // Begining of Vendor specific content + 4 ?random? bytes : element id
  { 0x15, 0, 6, 0x000000dd },   // jeq #0xdd jt 13 jf 19        // element id should be 221 (according to the doc)
  { 0x40, 0, 0, 0x00000021 },   // Ld  [x + 33]             // OUI (again!) on 3 LS bytes
  { 0x54, 0, 0, 0x00ffffff },   // and #0xffffff            // Mask the 3 LS bytes
  { 0x15, 0, 3, 0x0018fe34 },   // jeq #0x18fe34 jt 16 jf 19        // Compare with OUI 18:fe:34
  { 0x50, 0, 0, 0x00000025 },   // ldb [x + 37]             // Type
  { 0x15, 0, 1, 0x00000004 },   // jeq #0x4 jt 18 jf 19         // Compare type with type 0x4 (corresponding to ESP_NOW)
  { 0x6, 0, 0, 0x00040000 },    // ret #262144  // return 'True'
  { 0x6, 0, 0, 0x00000000 },    // ret #0   // return 'False'
};

void print_mac(uint8_t mac[6]) 
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void print_packet(uint8_t *data, int len)
{
    printf("----------------------------new packet-----------------------------------\n");
    int i;

    long my_packet_len = sizeof(espnow_packet_t);
    printf("expected packet len: %ld\n", my_packet_len);

    if (len != my_packet_len) {
        for (i = 0; i < len; i++) {
            if (i % 16 == 0)
                printf("\n");
            printf("%02x ", data[i]);
        }
        printf("\n\n");
        return;
    }

    espnow_packet_t my_packet = *(espnow_packet_t*) data;
    printf("packet from ");
    print_mac(my_packet.wlan.sa);
    printf(" to ");
    print_mac(my_packet.wlan.da);
    printf("\n");

    printf("data:");
    for (i = 0; i < DATA_LEN; i++) {
        if (i % 16 == 0)
            printf("\n");
        printf("0x%02x ", my_packet.wlan.actionframe.content.payload[i]);
    }
    printf("\n\n");
}

int create_raw_socket(char *dev, struct sock_fprog *bpf)
{
    struct sockaddr_ll sll;
    struct ifreq ifr;
    int fd, ifi, rb, attach_filter;

    bzero(&sll, sizeof(sll));
    bzero(&ifr, sizeof(ifr));

    fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    assert(fd != -1);

    strncpy((char *)ifr.ifr_name, dev, IFNAMSIZ);
    ifi = ioctl(fd, SIOCGIFINDEX, &ifr);
    assert(ifi != -1);

    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_pkttype = PACKET_OTHERHOST;

    rb = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    assert(rb != -1);

    attach_filter = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, bpf, sizeof(*bpf));
    assert(attach_filter != -1);

    return fd;
}

int main(int argc, char **argv)
{
    assert(argc == 2);

    nice(-20);

    uint8_t buff[MAX_PACKET_LEN] = {0};
    int sock_fd;
    char *dev = argv[1];
    struct sock_fprog bpf = {FILTER_LENGTH, bpfcode};

    sock_fd = create_raw_socket(dev, &bpf); /* Creating the raw socket */

    printf("sock_fd = %d\n", sock_fd);

    printf("\n Waiting to receive packets ........ \n");

    stopwatch_t timer_info, timer_start;
                 
    
    // int clock_res; // TODO: add error checking 
    clock_getres(CLOCK_REALTIME, &timer_info);
    
    printf("clock resolution info\ntv_sec = %ld\ntv_nsec = %ld\n",
           timer_info.tv_sec, timer_info.tv_nsec);

    clock_gettime(CLOCK_REALTIME, &timer_start);

    printf("start clock:\n");
    print_time(timer_start);
    
    struct storage s;
    zero_storage(&s);

    while (1)
    {
        int len = recvfrom(sock_fd, buff, MAX_PACKET_LEN, MSG_TRUNC, NULL, 0);

        process_packet(&s, buff, len);
    }
    close(sock_fd);
    return 0;
}
