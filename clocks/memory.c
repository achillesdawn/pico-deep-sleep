#include "hardware/structs/syscfg.h"
#include "hardware/structs/xip_ctrl.h"

void memory_powerdown() {
    // Turn off flash
    xip_ctrl_hw->ctrl |= XIP_CTRL_POWER_DOWN_BITS;
    // Turn off SRAMs
    syscfg_hw->mempowerdown = 0x00000020 | // SRAM5
                              0x00000010 | // SRAM4
                              0x00000080 | // ROM
                              0x00000040;  // USB
}

void memory_powerup() {
    // Turn on SRAMs
    syscfg_hw->mempowerdown = 0;
    // Turn on flash
    xip_ctrl_hw->ctrl &= ~XIP_CTRL_POWER_DOWN_BITS;
}
