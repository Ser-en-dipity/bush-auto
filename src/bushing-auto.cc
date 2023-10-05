#include <cstdint>
#include <cstring>
#include <functional>

#include "ctrl/fsmlist.hpp"
#include "ec/ec_ctx.h"
#include "ec/pdo_def.h"

using Rx = icnc::ecat::RxPdo_t;
using Tx = icnc::ecat::TxPdo_t;

auto main() -> int {
    fsm_list::start();

    std::function<void(Rx* const, const Tx* const)> e_cat_callback =
        [](Rx* const rx, const Tx* const tx) -> void {
        IOEvent io(rx, tx);
        SendEvent(io);
    };

    icnc::ecat::ECat ec(std::move(e_cat_callback));

    ec.wait();

    return 0;
}