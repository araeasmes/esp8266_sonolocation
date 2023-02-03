#ifndef _NET_INTERFACE_UTILS
#define _NET_INTERFACE_UTILS

#include <ifaddrs.h>
#include <netdb.h>
#include <sys/types.h>

#define MAX_ADDRESS_LEN NI_MAXHOST

void printNetworkInterfaces();
int findInterfaceAddress(const char *interface_name, char address[NI_MAXHOST]);

#endif // _NET_INTERFACE_UTILS
