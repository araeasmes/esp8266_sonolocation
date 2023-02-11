#ifndef _PAYLOAD_H
#define _PAYLOAD_H

#include <stdint.h>

struct payload_t {
    uint32_t cntr;
} __attribute__((__packed__));

#define DATA_LEN (sizeof(struct payload_t))


#endif // _PAYLOAD_H
