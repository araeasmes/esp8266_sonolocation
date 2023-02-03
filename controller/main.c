#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net_interface_utils.h"


int main(int argc, char **argv) {
    assert(argc > 1);        
    const char *interface_name = argv[1];
    char address[MAX_ADDRESS_LEN];
    explicit_bzero(address, MAX_ADDRESS_LEN);

    printNetworkInterfaces();
    int find_res = findInterfaceAddress(interface_name, address);

    if (find_res == 0)
        printf("address = %s\n", address);
    else 
        printf("failed to find the interface\n");

    exit(EXIT_SUCCESS);
}
