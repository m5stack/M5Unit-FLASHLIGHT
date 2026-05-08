/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Example using M5UnitUnified for UnitFlashLight (AW3641E)

  *********************************************************************
  Set the on-board mode selection switch (silkscreen `S1`) to match the
  demo you want to run, then choose the corresponding define below.
  Uncomment ONE of the lines (or pass via -D build flag) before flashing.
  *********************************************************************

  // Default: Flash mode (S1 = Flash side)
  // BtnA click: cycles through brightness 100%..30% with a short flash.
  //
  // To run the Torch demo, uncomment USE_TORCH_DEMO below or pass
  // -DUSE_TORCH_DEMO via build flags. Then set S1 = Torch side.
  // BtnA click: drives a 1000 ms continuous illumination at ~214 mA.
*/
// #define USE_TORCH_DEMO

#include <M5Unified.h>
#include <M5UnitUnified.h>
#include <M5UnitUnifiedFLASHLIGHT.h>

using namespace m5::unit::aw3641e;

namespace {
auto& lcd{M5.Display};
m5::unit::UnitUnified Units;
m5::unit::UnitFlashLight unit;

const Brightness cycle[] = {
    Brightness::Pct100, Brightness::Pct90, Brightness::Pct80, Brightness::Pct70,
    Brightness::Pct60,  Brightness::Pct50, Brightness::Pct40, Brightness::Pct30,
};
size_t idx{0};

#if defined(USE_TORCH_DEMO)
constexpr uint16_t DURATION_MAX_MS{TORCH_MAX_DURATION_MS};
#else
constexpr uint16_t DURATION_MAX_MS{FLASH_MAX_DURATION_MS};
#endif
constexpr uint16_t DURATION_MIN_MS{10};
uint16_t current_duration_ms{DURATION_MAX_MS};

struct IoPins {
    int rx;
    int tx;
};

IoPins get_gpio_pins()
{
    // Port.B on M5Core family (Core / Core2 / CoreS3 / Fire / Paper)
    auto rx{M5.getPin(m5::pin_name_t::port_b_in)};
    auto tx{M5.getPin(m5::pin_name_t::port_b_out)};
    if (rx >= 0 && tx >= 0) {
        return {rx, tx};
    }
    // Fallback to Port.A (StickC / Atom / Stamp / NanoC6 etc.)
    // FlashLight only needs the Yellow wire; map it to port_a_sda as TX.
    rx = M5.getPin(m5::pin_name_t::port_a_scl);
    tx = M5.getPin(m5::pin_name_t::port_a_sda);
    return {rx, tx};
}

unsigned brightness_percent(const Brightness b)
{
    // Pct100=0, Pct90=1, ..., Pct30=7 -> 100 - value * 10
    return 100u - static_cast<unsigned>(static_cast<uint8_t>(b)) * 10u;
}

void draw_status(const char* last_action)
{
    // Skip drawing on boards without a display (Atom / AtomLite / StampS3 / NanoC6 etc.)
    if (lcd.width() <= 0 || lcd.height() <= 0) {
        return;
    }
    const bool wide{lcd.width() >= 240};

    lcd.fillScreen(TFT_BLUE);
    lcd.setCursor(0, 0);
    lcd.setTextColor(TFT_WHITE, TFT_BLUE);

#if defined(USE_TORCH_DEMO)
    if (wide) {
        lcd.printf("FlashLight Demo\n");
        lcd.printf("S1: Torch side\n");
        lcd.printf("dur:        %ums\n", static_cast<unsigned>(current_duration_ms));
        lcd.printf("last:       %s\n", last_action ? last_action : "-");
        lcd.printf("\nBtnA click: torch\n");
        lcd.printf("BtnA hold : dur step\n");
    } else {
        lcd.printf("FlashLight\n");
        lcd.printf("S1: Torch\n");
        lcd.printf("dur: %ums\n", static_cast<unsigned>(current_duration_ms));
        lcd.printf("act: %s\n", last_action ? last_action : "-");
        lcd.printf("\nA clk: torch\n");
        lcd.printf("A hld: step\n");
    }
#else
    if (wide) {
        lcd.printf("FlashLight Demo\n");
        lcd.printf("S1: Flash side\n");
        lcd.printf("idx:        %u\n", static_cast<unsigned>(idx));
        lcd.printf("brightness: %u%%\n", brightness_percent(cycle[idx]));
        lcd.printf("dur:        %ums\n", static_cast<unsigned>(current_duration_ms));
        lcd.printf("last:       %s\n", last_action ? last_action : "-");
        lcd.printf("\nBtnA click: flash\n");
        lcd.printf("BtnA hold : dur step\n");
    } else {
        lcd.printf("FlashLight\n");
        lcd.printf("S1: Flash\n");
        lcd.printf("idx: %u\n", static_cast<unsigned>(idx));
        lcd.printf("bri: %u%%\n", brightness_percent(cycle[idx]));
        lcd.printf("dur: %ums\n", static_cast<unsigned>(current_duration_ms));
        lcd.printf("act: %s\n", last_action ? last_action : "-");
        lcd.printf("\nA clk: flash\n");
        lcd.printf("A hld: step\n");
    }
#endif
}
}  // namespace

void setup()
{
    auto m5cfg{M5.config()};
    M5.begin(m5cfg);
    M5.setTouchButtonHeightByRatio(100);

    if (lcd.height() > lcd.width()) {
        lcd.setRotation(1);
    }

    auto cfg = unit.config();
#if defined(USE_TORCH_DEMO)
    cfg.switch_position = SwitchPosition::Torch;
#else
    cfg.switch_position = SwitchPosition::Flash;
#endif
    unit.config(cfg);

    const auto pins{get_gpio_pins()};
    M5.Log.printf("FlashLight pins: RX=%d TX=%d\n", pins.rx, pins.tx);
    if (pins.rx < 0 || pins.tx < 0) {
        M5_LOGE("No GPIO pins available for this board (%u)", static_cast<unsigned>(M5.getBoard()));
        lcd.fillScreen(TFT_RED);
        while (true) {
            M5.update();
            m5::utility::delay(100);
        }
    }

    if (!Units.add(unit, pins.rx, pins.tx) || !Units.begin()) {
        M5_LOGE("Failed to begin");
        lcd.fillScreen(TFT_RED);
        while (true) {
            M5.update();
            m5::utility::delay(100);
        }
    }

    M5.Log.printf("%s\n", Units.debugInfo().c_str());
    draw_status("-");
}

void loop()
{
    M5.update();
    Units.update();

    if (M5.BtnA.wasClicked()) {
#if defined(USE_TORCH_DEMO)
        if (unit.torch(current_duration_ms)) {
            M5.Log.printf("torch: duration=%ums\n", static_cast<unsigned>(current_duration_ms));
            draw_status("torch");
        }
#else
        const Brightness b{cycle[idx]};
        if (unit.flash(b, current_duration_ms)) {
            M5.Log.printf("flash: idx=%u brightness=%u%% duration=%ums\n", static_cast<unsigned>(idx),
                          brightness_percent(b), static_cast<unsigned>(current_duration_ms));
            idx = (idx + 1) % (sizeof(cycle) / sizeof(cycle[0]));
            draw_status("flash");
        }
#endif
    }

    if (M5.BtnA.wasHold()) {
        if (current_duration_ms == DURATION_MAX_MS) {
            current_duration_ms = DURATION_MIN_MS;
        } else {
            const uint32_t doubled{static_cast<uint32_t>(current_duration_ms) * 2u};
            current_duration_ms = (doubled >= DURATION_MAX_MS) ? DURATION_MAX_MS : static_cast<uint16_t>(doubled);
        }
        M5.Log.printf("duration step: %ums\n", static_cast<unsigned>(current_duration_ms));
        draw_status("step");
    }
}
