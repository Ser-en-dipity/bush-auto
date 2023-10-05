#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace icnc::ecat {
static constexpr int kEcRxPdoSize = 2;
static constexpr int kEcTxPdoSize = 2;

static constexpr int kDigiONo = kEcRxPdoSize;
static constexpr int kDigiINo = kEcTxPdoSize;

static constexpr int kEcPdoSize = kEcRxPdoSize + kEcTxPdoSize;

#ifdef __GNUC__
#pragma pack(push, 1)
#endif

using RxPdo_t = struct RxPdo { std::array<std::uint8_t, kEcRxPdoSize> o; };

using TxPdo_t = struct TxPdo { std::array<std::uint8_t, kEcTxPdoSize> i; };

#ifdef __GNUC__
#pragma pack(pop)
#endif
}  // namespace icnc::ecat
