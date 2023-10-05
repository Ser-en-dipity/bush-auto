#pragma once

#include <cstdint>
#include <memory>

// static constexpr std::uint16_t kI_SWITCH_AUTO   = 0x4000;  // 0b 0100 0000 0000 0000
// static constexpr std::uint16_t kI_SWITCH_RESET  = 0x2000;  // 0b 0001 0000 0000 0000
// static constexpr std::uint16_t kI_SWITCH_MANUAL = 0x0000;  // 0b 0010 0000 0000 0000

static constexpr std::uint16_t kEMERGENCY_STOP  = 0x8000;  // 0b 1000 0000 0000 0000
static constexpr std::uint16_t kI_SWITCH        = 0x6000;  // 0b 0110 0000 0000 0000
static constexpr std::uint16_t kI_SWITCH_AUTO   = 0x4000;
static constexpr std::uint16_t kI_SWITCH_MANUAL = 0x2000;
static constexpr std::uint16_t kI_SWITCH_RESET  = 0x0000;
static constexpr std::uint16_t kI_OPERATE       = 0x1000;  // 0b 0000 1000 0000 0000

static constexpr std::uint16_t kI_M10 = 0x0800;  // 0b 0000 0100 0000 0000

static constexpr std::uint16_t kI_C1_PASS = 0x0400;

static constexpr std::uint16_t kI_CHUNK_ON  = 0x0200;
static constexpr std::uint16_t kI_CHUNK_OFF = 0x0100;

static constexpr std::uint16_t kI_C4_ON  = 0x0080;
static constexpr std::uint16_t kI_C4_OFF = 0x0040;
static constexpr std::uint16_t kI_C3_ON  = 0x0020;
static constexpr std::uint16_t kI_C3_OFF = 0x0010;
static constexpr std::uint16_t kI_C2_ON  = 0x0008;
static constexpr std::uint16_t kI_C2_OFF = 0x0004;
static constexpr std::uint16_t kI_C5_ON  = 0x0002;
static constexpr std::uint16_t kI_C5_OFF = 0x0001;
static constexpr std::uint16_t kINPUT    = 0x00FF;  // 0b 0000 0000 1111 1111

static constexpr std::uint16_t kO_C1    = 0x0001;
static constexpr std::uint16_t kO_C2    = 0x0002;
static constexpr std::uint16_t kO_C3    = 0x0004;
static constexpr std::uint16_t kO_C4    = 0x0008;
static constexpr std::uint16_t kO_C5    = 0x0010;
static constexpr std::uint16_t kO_CHUCK = 0x0020;
static constexpr std::uint16_t kO_SAFE  = 0x0040;
