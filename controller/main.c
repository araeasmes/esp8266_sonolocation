#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include "net_interface_utils.h"


int main(int argc, char **argv) {
    // assert(argc > 1);        
    // const char *interface_name = argv[1];

    printNetworkInterfaces();

    exit(EXIT_SUCCESS);
}
