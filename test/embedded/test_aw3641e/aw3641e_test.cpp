/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*
  UnitTest for UnitAW3641E (Unit FlashLight)
*/
#include <gtest/gtest.h>
#include <M5Unified.h>
#include <M5UnitUnified.hpp>
#include <googletest/test_template.hpp>
#include <googletest/test_helper.hpp>
#include <unit/unit_AW3641E.hpp>
#include <wiring/m5_unit_unified_wiring.hpp>

using namespace m5::unit::googletest;
using namespace m5::unit;
using namespace m5::unit::aw3641e;

class TestAW3641EBase : public GPIOComponentTestBase<UnitAW3641E> {
protected:
    // Override to match UnitAW3641E's TX-only nature (EN pin only, no RX).
    bool begin() override
    {
        return m5::unit::wiring::addGPIO(Units, *unit, m5::unit::wiring::GpioRole::OutOnly) && Units.begin();
    }
};

class TestAW3641E : public TestAW3641EBase {
protected:
    virtual UnitAW3641E* get_instance() override
    {
        return new m5::unit::UnitAW3641E();
    }
};

class TestAW3641EAsTorch : public TestAW3641EBase {
protected:
    virtual UnitAW3641E* get_instance() override
    {
        auto* u             = new m5::unit::UnitAW3641E();
        auto cfg            = u->config();
        cfg.switch_position = SwitchPosition::Torch;
        u->config(cfg);
        return u;
    }
};

// --- HW-in-loop tests (require GPIO adapter to be wired to U152) ---

TEST_F(TestAW3641E, Instance)
{
    SCOPED_TRACE(ustr);
    EXPECT_NE(unit, nullptr);
}

TEST_F(TestAW3641E, BeginInitialState)
{
    SCOPED_TRACE(ustr);
    // Default config: switch_position = Flash, no flash/torch active
    EXPECT_FALSE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 0u);
    EXPECT_EQ(unit->config().switch_position, SwitchPosition::Flash);
}

TEST_F(TestAW3641E, FlashShortPulse)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 50));
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 50u);

    // Let auto-off trigger via update()
    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, FlashWithDefaults)
{
    SCOPED_TRACE(ustr);

    // flash() with no args should default to Brightness::Pct100, duration FLASH_MAX_DURATION_MS
    EXPECT_TRUE(unit->flash());
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), FLASH_MAX_DURATION_MS);

    m5::utility::delay(FLASH_MAX_DURATION_MS + 30);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, FlashRejectsZeroDuration)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->flash(Brightness::Pct100, 0));
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, FlashDurationClamp)
{
    SCOPED_TRACE(ustr);

    // duration > FLASH_MAX_DURATION_MS is clamped
    EXPECT_TRUE(unit->flash(Brightness::Pct100, FLASH_MAX_DURATION_MS + 500));
    EXPECT_EQ(unit->lastDurationMs(), FLASH_MAX_DURATION_MS);
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641E, AllBrightnessLevels)
{
    SCOPED_TRACE(ustr);

    const Brightness levels[] = {Brightness::Pct100, Brightness::Pct90, Brightness::Pct80, Brightness::Pct70,
                                 Brightness::Pct60,  Brightness::Pct50, Brightness::Pct40, Brightness::Pct30};
    for (auto b : levels) {
        EXPECT_TRUE(unit->flash(b, 30));
        EXPECT_TRUE(unit->active());
        EXPECT_EQ(unit->lastDurationMs(), 30u);
        m5::utility::delay(50);
        unit->update();
        EXPECT_FALSE(unit->active());
    }
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641E, FlashCancelsInFlightWithStop)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 200));
    EXPECT_TRUE(unit->active());

    EXPECT_TRUE(unit->stop());
    EXPECT_FALSE(unit->active());

    // update() should not flip active back on.
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, FlashRetriggerCancelsPrevious)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 200));
    EXPECT_TRUE(unit->active());

    // Re-trigger while still active should cancel the previous and start new.
    EXPECT_TRUE(unit->flash(Brightness::Pct50, 50));
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 50u);

    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, StopWhenIdleIsNoOpSuccess)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->active());
    EXPECT_TRUE(unit->stop());
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, FlashStaysActiveBeforeDuration)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 200));
    unit->update();  // well before 200 ms
    EXPECT_TRUE(unit->active());
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641E, RejectedCallKeepsFlashActive)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 200));
    EXPECT_FALSE(unit->flash(Brightness::Pct100, 0));  // rejected
    EXPECT_FALSE(unit->torch(50));                     // rejected (Flash position)
    EXPECT_FALSE(unit->torchOn());                     // rejected (Flash position)
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 200u);
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641E, LastDurationKeptAfterStopAndAutoOff)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 50));
    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 50u);

    EXPECT_TRUE(unit->flash(Brightness::Pct100, 100));
    EXPECT_TRUE(unit->stop());
    EXPECT_EQ(unit->lastDurationMs(), 100u);
}

TEST_F(TestAW3641E, TorchRejectedInFlashPosition)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->torch(50));
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641E, TorchOnRejectedInFlashPosition)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->torchOn());
    EXPECT_FALSE(unit->active());
}

// --- Torch fixture (config: switch_position = Torch) ---

TEST_F(TestAW3641EAsTorch, TorchInitialState)
{
    SCOPED_TRACE(ustr);
    EXPECT_EQ(unit->config().switch_position, SwitchPosition::Torch);
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchAutoOff)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torch(50));
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 50u);

    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchWithDefaults)
{
    SCOPED_TRACE(ustr);

    // torch() with no arg defaults to TORCH_DEFAULT_DURATION_MS
    EXPECT_TRUE(unit->torch());
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), TORCH_DEFAULT_DURATION_MS);
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641EAsTorch, TorchRejectsZeroDuration)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->torch(0));
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchLongDurationNotClamped)
{
    SCOPED_TRACE(ustr);

    constexpr uint32_t long_ms{60000};  // Longer than uint16_t-era 1300 ms cap
    EXPECT_TRUE(unit->torch(long_ms));
    EXPECT_EQ(unit->lastDurationMs(), long_ms);
    unit->update();
    EXPECT_TRUE(unit->active());
    EXPECT_TRUE(unit->stop());
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchOnStaysOnUntilStop)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torchOn());
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 0u);

    // update() never turns a torchOn() off
    m5::utility::delay(80);
    unit->update();
    EXPECT_TRUE(unit->active());

    EXPECT_TRUE(unit->stop());
    EXPECT_FALSE(unit->active());
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchOnWhileOn)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torchOn());
    EXPECT_TRUE(unit->torchOn());
    EXPECT_TRUE(unit->active());
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641EAsTorch, TorchOnTakesOverTimedTorch)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torch(50));
    EXPECT_TRUE(unit->torchOn());
    EXPECT_EQ(unit->lastDurationMs(), 0u);

    // The timed turn-off of torch(50) no longer applies
    m5::utility::delay(80);
    unit->update();
    EXPECT_TRUE(unit->active());
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641EAsTorch, TimedTorchAfterTorchOn)
{
    SCOPED_TRACE(ustr);

    // torch() cancels torchOn() and turns off after its duration again
    EXPECT_TRUE(unit->torchOn());
    EXPECT_TRUE(unit->torch(50));
    EXPECT_EQ(unit->lastDurationMs(), 50u);

    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchRetriggerCancelsPrevious)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torch(50));
    m5::utility::delay(30);
    EXPECT_TRUE(unit->torch(100));  // restarts the timer
    EXPECT_EQ(unit->lastDurationMs(), 100u);
    m5::utility::delay(40);  // 70 ms after the first torch(50)
    unit->update();
    EXPECT_TRUE(unit->active());
    m5::utility::delay(80);
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, TorchCancelsInFlightWithStop)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torch(200));
    EXPECT_TRUE(unit->stop());
    EXPECT_FALSE(unit->active());
    unit->update();
    EXPECT_FALSE(unit->active());
}

TEST_F(TestAW3641EAsTorch, RejectedCallKeepsTorchActive)
{
    SCOPED_TRACE(ustr);

    EXPECT_TRUE(unit->torch(200));
    EXPECT_FALSE(unit->flash(Brightness::Pct100, 50));  // rejected (Torch position)
    EXPECT_FALSE(unit->torch(0));                       // rejected
    EXPECT_TRUE(unit->active());
    EXPECT_EQ(unit->lastDurationMs(), 200u);
    EXPECT_TRUE(unit->stop());
}

TEST_F(TestAW3641EAsTorch, FlashRejectedInTorchPosition)
{
    SCOPED_TRACE(ustr);

    EXPECT_FALSE(unit->flash(Brightness::Pct100, 50));
    EXPECT_FALSE(unit->active());
}
