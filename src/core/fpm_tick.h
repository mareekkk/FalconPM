// src/core/fpm_tick.h
// FalconPM tick utility — provides global millisecond timer

#ifndef FPM_TICK_H
#define FPM_TICK_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return current server tick in milliseconds.
 * Used by Hunter and other AI modules to measure cooldowns, delays, etc.
 */
uint64_t fpm_gettick();

#ifdef __cplusplus
}
#endif

#endif // FPM_TICK_H
