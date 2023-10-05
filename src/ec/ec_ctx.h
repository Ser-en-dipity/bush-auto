#pragma once
#include <functional>
#include <thread>

#include "ec/pdo_def.h"

namespace icnc::ecat {
class ECat {
   public:
    explicit ECat(std::function<void(RxPdo_t* const, const TxPdo_t* const)>);
    ~ECat();

   public:
    ECat(const ECat&) = delete;
    ECat()            = delete;
    auto operator=(const ECat&) -> ECat& = delete;

   private:
    std::array<std::uint8_t, kEcPdoSize>                      iomap_;
    bool                                                      stop_flag_;
    ecat::RxPdo_t* const                                      ec_rx_pdo_;
    const ecat::TxPdo_t* const                                ec_tx_pdo_;
    std::function<void(RxPdo_t* const, const TxPdo_t* const)> cb_;
    std::thread                                               cyclic_worker_;

   public:
    void wait();
};
}  // namespace icnc::ecat
