#include "net_interface_utils.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


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
            struct rtnl_link_stats *stats = ifa->ifa_data;

            printf("\t\ttx_packets = %10u; rx_packets = %10u\n"
                   "\t\ttx_bytes   = %10u;  rx_bytes  = %10u\n",
                   stats->tx_packets, stats->rx_packets,
                   stats->tx_bytes, stats->rx_bytes);
        }
    }

    freeifaddrs(ifaddr);
}

int findInterfaceAddress(const char *interface_name, char address[NI_MAXHOST]) {
    struct ifaddrs *ifaddr;
    int family, s;

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
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                    (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                          sizeof(struct sockaddr_in6),
                                          address, NI_MAXHOST,
                                          NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                fprintf(stderr, "getnameinfo() failed: %s\n", gai_strerror(s));
                return -1;
            }
            return 0;
        }
    } 

    // interface with the same name not found
    return -1;
}


