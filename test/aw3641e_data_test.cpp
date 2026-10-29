/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  Tests for the AW3641E constants, pulse count translation and config (no hardware).
  Placed directly under test/, so they are built into every test suite
*/
#include <gtest/gtest.h>
#include <unit/unit_AW3641E.hpp>

using namespace m5::unit;
using namespace m5::unit::aw3641e;

TEST(AW3641E, PulseCountTranslation)
{
    EXPECT_EQ(to_pulse_count(Brightness::Pct100), 1);
    EXPECT_EQ(to_pulse_count(Brightness::Pct90), 2);
    EXPECT_EQ(to_pulse_count(Brightness::Pct80), 3);
    EXPECT_EQ(to_pulse_count(Brightness::Pct70), 4);
    EXPECT_EQ(to_pulse_count(Brightness::Pct60), 5);
    EXPECT_EQ(to_pulse_count(Brightness::Pct50), 6);
    EXPECT_EQ(to_pulse_count(Brightness::Pct40), 7);
    EXPECT_EQ(to_pulse_count(Brightness::Pct30), 8);
}

TEST(AW3641E, ConfigDefaults)
{
    UnitAW3641E::config_t cfg{};
    EXPECT_EQ(cfg.switch_position, SwitchPosition::Flash);
}

TEST(AW3641E, ConfigRoundTrip)
{
    UnitAW3641E unit{};

    auto cfg{unit.config()};
    cfg.switch_position = SwitchPosition::Torch;
    unit.config(cfg);
    EXPECT_EQ(unit.config().switch_position, SwitchPosition::Torch);
}

TEST(AW3641E, DurationConstants)
{
    EXPECT_EQ(FLASH_MAX_DURATION_MS, 220u);
    EXPECT_EQ(TORCH_DEFAULT_DURATION_MS, 1300u);
    EXPECT_LT(FLASH_MAX_DURATION_MS, TORCH_DEFAULT_DURATION_MS);
}

// The deprecated alias must keep the same value until it is removed
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
TEST(AW3641E, DeprecatedTorchMaxDurationAlias)
{
    EXPECT_EQ(TORCH_MAX_DURATION_MS, TORCH_DEFAULT_DURATION_MS);
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

TEST(AW3641E, PulseTimingConstants)
{
    // T_HI/T_LO must be within the AW3641E datasheet budget (0.75..10 us).
    // Lower bound relaxed to 1 us because constexpr uint32_t cannot express 0.75.
    EXPECT_GE(PULSE_HIGH_US, 1u);
    EXPECT_LE(PULSE_HIGH_US, 10u);
    EXPECT_GE(PULSE_LOW_US, 1u);
    EXPECT_LE(PULSE_LOW_US, 10u);
    // T_OFF must exceed 500 us per datasheet.
    EXPECT_GT(PULSE_OFF_US, 500u);
}
