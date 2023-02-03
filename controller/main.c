#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
    assert(argc > 1);        
    const char *interface_name = argv[1];

    struct ifaddrs *ifaddr;
    int family, s;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
            ifa = ifa->ifa_next) {

        printf("%s\n", ifa->ifa_name);     
    }


    freeifaddrs(ifaddr);
    exit(EXIT_SUCCESS);
}
