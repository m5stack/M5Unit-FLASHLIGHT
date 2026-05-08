/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
/*!
  @file M5UnitUnifiedFLASHLIGHT.hpp
  @brief Aggregates flashlight-related M5 Unit drivers and provides convenience aliases.

  Supported products:
  - UnitFlashLight (SKU:U152) — AW3641E 1-wire flash LED driver (GPIO)
*/
#ifndef M5_UNIT_UNIFIED_FLASHLIGHT_HPP
#define M5_UNIT_UNIFIED_FLASHLIGHT_HPP

#include "unit/unit_AW3641E.hpp"

/*!
  @namespace m5
  @brief Top level namespace of M5Stack
*/
namespace m5 {

/*!
  @namespace unit
  @brief Unit-related namespace
*/
namespace unit {

using UnitFlashLight = UnitAW3641E;  //!< Unit FlashLight (SKU:U152) — AW3641E 1-wire flash LED

}  // namespace unit
}  // namespace m5

#endif
