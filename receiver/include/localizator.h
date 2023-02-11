#ifndef _LOCALIZATOR_H
#define _LOCALIZATOR_H

#include <math.h>
#include <time.h>

#include "vecmath.h"

typedef struct timespec stopwatch_t;

struct sound_entry {
    int32_t mcu_ind;
    stopwatch_t timestamp;
    uint32_t cntr;
};


static inline void print_time(stopwatch_t timer)
{
    printf("%lds %ldns\n", timer.tv_sec, timer.tv_nsec);
}

#define NANO_MUL (1000 * 1000 * 1000)

stopwatch_t float_to_time(float t) {
    stopwatch_t res;
    res.tv_sec = truncf(t);
    res.tv_nsec = (t - truncf(t)) * NANO_MUL;
    return res;
}

float time_to_float(stopwatch_t t) {
    float res = 0.0f;
    res = t.tv_sec;
    res += t.tv_nsec / NANO_MUL;

    return res;
}

stopwatch_t time_diff(stopwatch_t timer_end, stopwatch_t timer_start);

#define STORAGE_STEP 256 

struct storage {
    struct sound_entry *data;
    uint32_t cnt;
    uint32_t size;
}; 

void zero_storage(struct storage *s);
// todo: change return type and error check
void add_entry(struct storage *s, struct sound_entry entry); 
void clean_storage(struct storage *s); 

#define SOUND_SPEED 340.0f
#define MCU_NUM 3

void match_signals(struct storage *s) 
{
    // dist[i] is distance between mcus i+1 and i+2 (modulo 3)
    float dists[MCU_NUM] = {M_SQRT2, 1.0f, 1.0f}; 

    float time_lims_sec[MCU_NUM];
    for (int i = 0; i < MCU_NUM; i++) {
        time_lims_sec[MCU_NUM] = dists[i] / SOUND_SPEED;
    }

    uint32_t packet_cntr[MCU_NUM] = {0, 0, 0};

    // try to match packets from 3 MCUs
    for (int i = 0; i < s->cnt; i++) {
        packet_cntr[s->data[i].mcu_ind]++; 
    }

    for (int i = 0; i < MCU_NUM; i++) {
        printf("mcu[%d] sent %u packets\n", i, packet_cntr[i]);
    }

    printf("-----===========-----\n");
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

void zero_storage(struct storage *s) 
{
    s->data = NULL;
    s->cnt = 0;
    s->size = 0;
}

void add_entry(struct storage *s, struct sound_entry entry) 
{
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

void clean_storage(struct storage *s) 
{
    free(s->data);
    s->data = NULL;
    s->cnt = 0;
    s->size = 0;
}

#endif

