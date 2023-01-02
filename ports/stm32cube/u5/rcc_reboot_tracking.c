//! @file
//!
//! Copyright (c) Memfault, Inc.
//! See License.txt for details
//!
//! A port for recovering reset reason information by reading the
//! "Reset and Clock Control" (RCC)'s "control & status register" (CSR) Register.
//!
//! More details can be found in the "RCC clock control & status register (RCC_CSR)"
//! section of the STM32L4 family reference manual.

#include "memfault/ports/reboot_reason.h"

#include "stm32u5xx_ll_rcc.h"

#include "memfault/config.h"
#include "memfault/core/debug_log.h"
#include "memfault/core/reboot_reason_types.h"
#include "memfault/core/sdk_assert.h"

#if MEMFAULT_ENABLE_REBOOT_DIAG_DUMP
#define MEMFAULT_PRINT_RESET_INFO(...) MEMFAULT_LOG_INFO(__VA_ARGS__)
#else
#define MEMFAULT_PRINT_RESET_INFO(...)
#endif

void memfault_reboot_reason_get(sResetBootupInfo *info) {
  MEMFAULT_SDK_ASSERT(info != NULL);

  const uint32_t reset_cause = RCC->CSR;

  eMemfaultRebootReason reset_reason = kMfltRebootReason_Unknown;

  MEMFAULT_LOG_INFO("Reset Reason, RCC_CSR=0x%" PRIx32, reset_cause);
  MEMFAULT_PRINT_RESET_INFO("Reset Causes: ");

  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SBF) != RESET) {
    MEMFAULT_PRINT_RESET_INFO(" Standby Wakeup");
    reset_reason = kMfltRebootReason_DeepSleep;
    // clear the standy wakeup
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
  } else if (reset_cause & RCC_CSR_LPWRRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Low Power Reset");
    reset_reason = kMfltRebootReason_UsageFault;
  } else if (reset_cause & RCC_CSR_SFTRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Software");
    reset_reason = kMfltRebootReason_SoftwareReset;
  } else if (reset_cause & RCC_CSR_IWDGRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Independent Watchdog");
    reset_reason = kMfltRebootReason_HardwareWatchdog;
  } else if (reset_cause & RCC_CSR_WWDGRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Window Watchdog");
    reset_reason = kMfltRebootReason_HardwareWatchdog;
  } else if (reset_cause & RCC_CSR_BORRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Brown out / POR");
    // The STM32L4 doesn't have a way to disambiguate
    // a BOR vs POR
    reset_reason = kMfltRebootReason_PowerOnReset;
  } else if (reset_cause & RCC_CSR_PINRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Pin Reset");
    reset_reason = kMfltRebootReason_PinReset;
  } else if (reset_cause & RCC_CSR_OBLRSTF) {
    MEMFAULT_PRINT_RESET_INFO(" Option Byte Loader");
    // Unexpected reset type, we'll just classify as Unknown
  } else {
    MEMFAULT_PRINT_RESET_INFO(" Unknown");
  }

  // we have read the reset information so clear the bits (since they are sticky across reboots)
  __HAL_RCC_CLEAR_RESET_FLAGS();

  *info = (sResetBootupInfo) {
    .reset_reason_reg = reset_cause,
    .reset_reason = reset_reason,
  };
}
