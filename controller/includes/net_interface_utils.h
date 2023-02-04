#ifndef _NET_INTERFACE_UTILS
#define _NET_INTERFACE_UTILS

#include <ifaddrs.h>
#include <netdb.h>
#include <sys/types.h>

#define MAX_ADDRESS_LEN NI_MAXHOST

void printMAC(const unsigned char *addr, int len);
void printNetworkInterfaces();
int findInterfaceMAC(const char *interface_name, 
                     unsigned char address[8], unsigned char *addr_len);

#endif // _NET_INTERFACE_UTILS
