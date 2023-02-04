#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_interface_utils.h"


int main(int argc, char **argv) {
    assert(argc > 1);        
    const char *interface_name = argv[1];

    unsigned char addr_len;
    unsigned char address[8];
    explicit_bzero(address, 8);

    printNetworkInterfaces();
    int find_res = findInterfaceMAC(interface_name, address, &addr_len);

    if (find_res == 0) {
        printMAC(address, addr_len);
        printf("\n");
     } else 
        printf("failed to find the interface\n");

    exit(EXIT_SUCCESS);
}
