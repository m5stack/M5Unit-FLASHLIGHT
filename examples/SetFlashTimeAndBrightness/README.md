# SetFlashTimeAndBrightness (legacy)

This is a **legacy** sample kept only for backward compatibility with the
original M5Stack example. It predates the M5UnitUnified-based driver and
should not be used as a template for new projects.

## Recommended alternative

For new projects, use
[`../UnitUnified/UnitFlashLight/FlashDemo`](../UnitUnified/UnitFlashLight/FlashDemo),
which drives the AW3641E through the maintained public API
(`unit.flash()` / `unit.torch()`) with proper timing and safety.

## Constraints

- Target board: **M5Stack Core only** (uses `M5Stack.h`)
- Flash EN pin: **GPIO 26 (fixed)**
- Uses raw `pinMode` / `digitalWrite` instead of the M5UnitUnified API

## Known defects (kept as-is)

- **T_OFF not implemented** — the pulse train starts with only a 4 µs LOW
  interval, whereas the AW3641E requires EN LOW for more than 500 µs to
  reset the previous command. Pressing the trigger repeatedly while EN is
  HIGH may append or misinterpret pulses instead of selecting the requested
  brightness.
- **No interrupt protection** — the pulse train runs without disabling
  interrupts, so FreeRTOS task switches under load can push T_HI / T_LO
  outside the 0.75-10 µs window and cause timing jitter.

## Purpose

Preserved as a reference to the original M5Stack example, so users who
depend on it are not broken by the library re-organization. The maintained
`FlashDemo` example is the recommended path for all new work.
