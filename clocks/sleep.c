#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/stdlib.h"

// For __wfi
#include "hardware/sync.h"
// For scb_hw so we can enable deep sleep
#include "clocks/clocks.h"
#include "hardware/structs/scb.h"
#include "sleep.h"

// Go to sleep until woken up by the RTC
void sleep_goto_sleep_until(datetime_t *t, rtc_callback_t callback) {
    sleep_state_t *sleep_state = sleep_state_save();
    // Turn off all clocks when in sleep mode except for RTC
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_RTC_RTC_BITS;
    clocks_hw->sleep_en1 = 0x0;

    rtc_set_alarm(t, callback);

    uint save = scb_hw->scr;
    // Enable deep sleep at the proc
    scb_hw->scr = save | M0PLUS_SCR_SLEEPDEEP_BITS;

    // Go to sleep
    __wfi();

    sleep_state_recover(sleep_state);
}

// Go to sleep until woken up by the RTC
void dormant_until(datetime_t *t, rtc_callback_t callback) {
    sleep_state_t *sleep_state = sleep_state_save();
    // Turn off all clocks when in sleep mode except for RTC
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_RTC_RTC_BITS;
    clocks_hw->sleep_en1 = 0x0;

    rtc_set_alarm(t, callback);

    uint save = scb_hw->scr;
    // Enable deep sleep at the proc
    scb_hw->scr = save | M0PLUS_SCR_SLEEPDEEP_BITS;

    dormant_xosc();

    sleep_state_recover(sleep_state);
}
