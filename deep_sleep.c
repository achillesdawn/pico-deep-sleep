#include <stdio.h>

#include "hardware/clocks.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
// For __wfi
#include "hardware/sync.h"
// For scb_hw so we can enable deep sleep
#include "hardware/structs/scb.h"

#include "clocks.h"

const uint8_t LED = 8;

volatile bool awake = false;

void rtc_callback_on_wake() {
    awake = true;
    gpio_put(LED, true);
    printf("woken up");
}

// Go to sleep until woken up by the RTC
void sleep_goto_sleep_until(datetime_t *t, rtc_callback_t callback) {

    // Turn off all clocks when in sleep mode except for RTC
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_RTC_RTC_BITS;
    clocks_hw->sleep_en1 = 0x0;

    rtc_set_alarm(t, callback);

    uint save = scb_hw->scr;
    // Enable deep sleep at the proc
    scb_hw->scr = save | M0PLUS_SCR_SLEEPDEEP_BITS;

    // Go to sleep
    __wfi();
}

int main() {
    stdio_init_all();

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, true);

    sleep_ms(2000);

    measure_freqs();

    for (uint8_t i = 0; i < 10; i++) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }

    printf(
        "SLEEP EN0:%d, SLEEP EN1: %d",
        clocks_hw->sleep_en0,
        clocks_hw->sleep_en1
    );

    rtc_init();

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

    datetime_t target_sleep;

    rtc_get_datetime(&target_sleep);

    target_sleep.sec = target_sleep.sec + 15;
    sleep_state_t *sleep_state = sleep_state_save();

    printf("going to sleep for 15 sec");
    gpio_put(LED, false);

    run_from_rosc(true);
    sleep_goto_sleep_until(&target_sleep, &rtc_callback_on_wake);

    while (!awake) {
        tight_loop_contents();
    }

    recover_from_sleep(sleep_state); 

    for (uint8_t i = 0; i < 20; i++) {
        gpio_put(LED, false);
        sleep_ms(100);
        gpio_put(LED, true);
        sleep_ms(100);
    }
}
