#include "ec/ec_ctx.h"

#include <unistd.h>

#include <array>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <utility>

#include "ethercat.h"
#include "pdo_def.h"

namespace icnc::ecat {
static constexpr int                 kCycleTime = 125 * 1000;
static constexpr int                 kNsPerS    = 1e9;
static constexpr std::array<char, 5> kEthName   = {"eth0"};
static inline void                   TimeIncrease(struct timespec*, std::int64_t);
static auto                          InitSoem(std::string&& eth_name, void* p_map) -> bool;

using RX = icnc::ecat::RxPdo_t;
using TX = icnc::ecat::TxPdo_t;

static void CyclicWorker(bool&,
                         RX* const,
                         const TX* const,
                         const std::function<void(RX* const, const TX* const)>&);

ECat::ECat(std::function<void(RX* const, const TX* const)> cb)
    : iomap_{0},
      stop_flag_(false),
      ec_rx_pdo_(reinterpret_cast<RX* const>(&iomap_[0])),
      ec_tx_pdo_(reinterpret_cast<const TxPdo_t* const>(&iomap_[kEcRxPdoSize])),
      cb_(std::move(cb)),
      cyclic_worker_(std::thread(CyclicWorker,
                                 std::ref(stop_flag_),
                                 ec_rx_pdo_,
                                 ec_tx_pdo_,
                                 std::ref(this->cb_))) {}

ECat::~ECat() {
    stop_flag_ = true;
    this->cyclic_worker_.join();
}
void ECat::wait() {
    this->stop_flag_ = false;
    this->cyclic_worker_.join();
}

static void CyclicWorker(bool&                                                  stop_flag,
                         RX* const                                              rx_pdo,
                         const TX* const                                        tx_pdo,
                         const std::function<void(RX* const, const TX* const)>& cb) {
#if !defined(ecat_debug)
    if (!InitSoem(kEthName.data(), reinterpret_cast<void*>(rx_pdo->o.data()))) {
        return;
    }
#endif

#if !defined(ecat_debug)
    struct timespec timer;
    clock_gettime(CLOCK_MONOTONIC, &timer);
#endif

    while (!stop_flag) {
#if !defined(ecat_debug)
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
#endif
        cb(rx_pdo, tx_pdo);
#if defined(ecat_debug)
        usleep(125000);
#else
        TimeIncrease(&timer, kCycleTime);
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timer, nullptr);
#endif
    }
}
static inline void TimeIncrease(struct timespec* timer, std::int64_t nanosec) {
    timer->tv_nsec += nanosec;
    while (timer->tv_nsec >= kNsPerS) {
        timer->tv_sec++;
        timer->tv_nsec -= kNsPerS;
    }
}

static auto InitSoem(std::string&& eth_name, void* p_map) -> bool {
    if (!ec_init(eth_name.c_str())) {
        return false;
    }
    if (ec_config_init(FALSE) <= 0) {
        return false;
    }
    if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE * 4) != EC_STATE_PRE_OP) {
        return false;
    }
#if !defined(ecat_debug)
    ec_configdc();
    for (int slc = 1; slc <= ec_slavecount; slc++) {
        if (ec_slave[slc].hasdc) {
            ec_dcsync0(slc, TRUE, kCycleTime, 0);
        }
    }
#endif
    ec_config_map(p_map);
    if (!ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4)) {
        return false;
    }

    ec_slave[0].state = EC_STATE_OPERATIONAL;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    ec_writestate(0);
    int chk = 40;
    do {
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);  // 50ms wait for state check
    } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

    if (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE) != EC_STATE_OPERATIONAL) {
        return false;
    }
    return true;
}

}  // namespace icnc::ecat
