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

volatile bool awake = false;

void rtc_callback_on_wake() {}

void setup() {
    stdio_init_all();

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, true);

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
    measure_freqs();

    for (uint8_t i = 0; i < 10; i++) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }

    gpio_put(LED, false);

    datetime_t target_sleep;

    rtc_get_datetime(&target_sleep);

    target_sleep.sec = target_sleep.sec + 15;

    run_from_rosc();
    // run_from_rosc_with_usb();

    // measure_freqs();
    // run_from_xosc(1 * MHZ);
    // run_from_xosc_rtc_rosc(6 * MHZ);

    printf("going to sleep for 15 sec");

    sleep_goto_sleep_until(&target_sleep, &rtc_callback_on_wake);
    // dormant_until(&target_sleep, &rtc_callback_on_wake);

    while (!awake) {
        tight_loop_contents();
    }

    for (uint8_t i = 0; i < 20; i++) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }
}
