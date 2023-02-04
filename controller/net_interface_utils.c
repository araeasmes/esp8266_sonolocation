#include "net_interface_utils.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


void printMAC(const unsigned char *addr, int len) {
    if (len <= 0)
        return;
    int i;
    for (i = 0; i < len - 1; i++) {
        printf("%02x:", addr[i]);
    }
    printf("%02x", addr[len - 1]);
}

void printNetworkInterfaces() {
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
            ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;
        
        family = ifa->ifa_addr->sa_family;

        printf("%-8s %s (%d)\n",
                ifa->ifa_name,
                (family == AF_PACKET) ? "AF_PACKET" : 
                (family == AF_INET) ? "AF_INET" : 
                (family == AF_INET6) ? "AF_INET6" : "family unkonwn",
                family);

        // display interface address
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                          sizeof(struct sockaddr_in6),
                                          host, NI_MAXHOST,
                                          NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                return;
            }

            printf("\t\taddress: <%s>\n", host);
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            // display the mac (?)
            struct sockaddr_ll *sll = (struct sockaddr_ll*) ifa->ifa_addr;
            printMAC(sll->sll_addr, sll->sll_halen);
            printf("\n");

            struct rtnl_link_stats *stats = ifa->ifa_data;
            printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                   "\t\ttx_bytes   = %10u;  rx_bytes  = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs(ifaddr);
}

int findInterfaceMAC(const char *interface_name, 
                     unsigned char address[8], unsigned char *addr_len) {
    struct ifaddrs *ifaddr;
    int family;
    struct sockaddr_ll *sll;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (strcmp(interface_name, ifa->ifa_name) != 0)
            continue;

        family = ifa->ifa_addr->sa_family;
        if (family == AF_PACKET) {
            sll = (struct sockaddr_ll*) ifa->ifa_addr;
            
            // is this check even needed?
            if (sll->sll_halen == 0)
                continue;

            *addr_len = sll->sll_halen; 
            for (int i = 0; i < sll->sll_halen; i++)
                address[i] = sll->sll_addr[i];

            return 0;
        }
    } 

    // interface with the same name not found
    return -1;
}


