#include <stdbool.h>

void measure_freqs(void);
bool set_peripheral_clk();
bool set_usb_clk();
void run_from_rosc(bool disable_xosc);
void run_from_xosc(uint32_t requested_freq);
void rosc_dormant();
bool clocks_reinit();

typedef struct sleep_state_t {
    uint32_t sleep_en0;
    uint32_t sleep_en1;
    uint32_t scr;
} sleep_state_t;

sleep_state_t *sleep_state_save();

void recover_from_sleep(sleep_state_t *sleep_state);

#pragma once
