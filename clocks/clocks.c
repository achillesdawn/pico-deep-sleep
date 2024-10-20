#include "clocks.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "hardware_rosc/rosc.h"
#include "pico/runtime_init.h"
#include "pico/stdlib.h"

void measure_freqs(void) {
    uint f_pll_sys =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb =
        frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);

    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);

    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);

    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);

    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("pll_sys  = %dkHz\n", f_pll_sys);
    printf("pll_usb  = %dkHz\n", f_pll_usb);
    printf("rosc     = %dkHz\n", f_rosc);
    printf("clk_sys  = %dkHz\n", f_clk_sys);
    printf("clk_peri = %dkHz\n", f_clk_peri);
    printf("clk_usb  = %dkHz\n", f_clk_usb);
    printf("clk_adc  = %dkHz\n", f_clk_adc);
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);

    uart_default_tx_wait_blocking();
    // Can't measure clk_ref / xosc as it is the ref
}

bool set_peripheral_clk() {
    clock_configure(clk_peri, 0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * 1000,
                    48 * 1000);

    stdio_init_all();
}

bool set_usb_clk() {
    clock_configure(clk_peri, 0,
                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB, 48 * KHZ,
                    48 * KHZ);

    stdio_init_all();
}

void run_from_rosc() {
    // requested frequency cannot be greater that source frequency, only divided
    uint32_t src_hz = 6.5 * MHZ;
    uint32_t requested_freq = src_hz;

    // CLK_REF = ROSC
    clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH, 0,
                    src_hz, requested_freq);

    // CLK SYS = CLK REF
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0,
                    requested_freq, requested_freq);

    clock_stop(clk_adc);
    clock_stop(clk_usb);

    clock_configure(clk_rtc,
                    0,  // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH, src_hz,
                    46875);

    clock_stop(clk_peri);

    pll_deinit(pll_sys);
    pll_deinit(pll_usb);

    xosc_disable();
}

void run_from_rosc_with_usb() {
    // requested frequency cannot be greater that source frequency, only divided
    uint32_t src_hz = 6.5 * MHZ;
    uint32_t requested_freq = src_hz;

    // CLK_REF = ROSC
    clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH, 0,
                    src_hz, requested_freq);

    // CLK SYS = CLK REF
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0,
                    requested_freq, requested_freq);

    clock_stop(clk_adc);
    // clock_stop(clk_usb);

    clock_configure(clk_rtc,
                    0,  // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH, src_hz,
                    46875);

    clock_stop(clk_peri);

    pll_deinit(pll_sys);
    // pll_deinit(pll_usb);
    setup_default_uart();

    xosc_disable();
}

void run_from_xosc_rtc_rosc(uint32_t requested_freq) {
    // requested frequency cannot be greater that source frequency, only divided
    uint32_t src_hz = XOSC_MHZ * MHZ;

    // CLK_REF = XOSC
    clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0,
                    src_hz, requested_freq);

    // CLK SYS = CLK REF
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0,
                    requested_freq, requested_freq);

    clock_stop(clk_adc);
    clock_stop(clk_usb);

    // set the RTC to run from ROSC
    clock_configure(clk_rtc,
                    0,  // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_ROSC_CLKSRC_PH, src_hz,
                    46875);

    clock_stop(clk_peri);
    pll_deinit(pll_usb);
    pll_deinit(pll_sys);

    // rosc_disable();
}

void run_from_xosc(uint32_t requested_freq) {
    // requested frequency cannot be greater that source frequency, only divided
    uint32_t src_hz = XOSC_MHZ * MHZ;

    // CLK_REF = XOSC
    clock_configure(clk_ref, CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC, 0,
                    src_hz, requested_freq);

    // CLK SYS = CLK REF
    clock_configure(clk_sys, CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF, 0,
                    requested_freq, requested_freq);

    clock_stop(clk_adc);
    clock_stop(clk_usb);

    clock_configure(clk_rtc,
                    0,  // No GLMUX
                    CLOCKS_CLK_RTC_CTRL_AUXSRC_VALUE_XOSC_CLKSRC, src_hz,
                    46875);

    clock_stop(clk_peri);
    pll_deinit(pll_usb);
    pll_deinit(pll_sys);

    rosc_disable();
}

void dormant_rosc() { rosc_set_dormant(); }

void dormant_xosc() { xosc_dormant(); }

void set_sleep_enabled_bits() {
    clocks_hw->sleep_en0 = CLOCKS_SLEEP_EN0_CLK_RTC_RTC_BITS;
    clocks_hw->sleep_en1 = 0x0;
}

sleep_state_t *sleep_state_save() {
    sleep_state_t *sleep_state = malloc(sizeof(sleep_state_t));
    if (sleep_state == NULL) {
        printf("could not allocate sleep state");
        return NULL;
    }

    sleep_state->scr = scb_hw->scr;
    sleep_state->sleep_en0 = clocks_hw->sleep_en0;
    sleep_state->sleep_en1 = clocks_hw->sleep_en1;
    return sleep_state;
}

void sleep_state_recover(sleep_state_t *sleep_state) {
    // Re-enable ring Oscillator control
    rosc_write(&rosc_hw->ctrl, ROSC_CTRL_ENABLE_BITS);

    // reset procs back to default
    scb_hw->scr = sleep_state->scr;
    clocks_hw->sleep_en0 = sleep_state->sleep_en0;
    clocks_hw->sleep_en1 = sleep_state->sleep_en1;

    // reset clocks
    clocks_init();
    stdio_init_all();

    return;
}
