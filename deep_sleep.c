#include <stdio.h>

#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "pico.h"
#include "pico/stdlib.h"

// For __wfi
#include "hardware/sync.h"
// For scb_hw so we can enable deep sleep
#include "clocks/clocks.h"
#include "clocks/sleep.h"
#include "hardware/structs/scb.h"

const uint8_t LED = 8;
const uint8_t RED_LED = 12;
const uint32_t OUT_MASK = (1 << LED) | (1 << RED_LED);

void rtc_callback_on_wake() {}

void setup() {
    stdio_init_all();

    gpio_init_mask(OUT_MASK);
    gpio_set_dir_out_masked(OUT_MASK);
    gpio_put_masked(OUT_MASK, false);

    rtc_init();
}

void set_datetime() {
    datetime_t t = {
        .year = 2024,
        .month = 10,
        .day = 18,
        .dotw = 5,
        .hour = 17,
        .min = 0,
        .sec = 00,
    };

    rtc_set_datetime(&t);

    // clk_sys is >2000x faster than clk_rtc, so datetime is not updated
    // immediately when rtc_get_datetime() is called. The delay is up to 3 RTC
    // clock cycles (which is 64us with the default clock settings)
    sleep_us(64);
}

int main() {
    setup();

    for (uint8_t i = 0; i < 10; i++) {
        gpio_put(LED, true);
        sleep_ms(100);
        gpio_put(LED, false);
        sleep_ms(100);
    }

    set_datetime();
    datetime_t sleep_target;

    rtc_get_datetime(&sleep_target);

    sleep_target.sec = sleep_target.sec + 10;

    printf("going to sleep for 15 sec");

    run_from_rosc();

    // run_from_rosc_with_usb();

    // measure_freqs();
    // run_from_xosc(1 * MHZ);
    // run_from_xosc_rtc_rosc(6 * MHZ);

    sleep_goto_sleep_until(&sleep_target);

    // dormant_until(&target_sleep, &rtc_callback_on_wake);

    for (uint8_t i = 0; i < 20; i++) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }

    while (true) {
        tight_loop_contents();
    }
}
