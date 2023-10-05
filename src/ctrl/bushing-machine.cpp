#include "bushing-machine.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <system_error>

#include "cylinder.hpp"
#include "fsmlist.hpp"
#include "masks.h"

class Initial;

auto RisingEdge(std::uint16_t last_input, std::uint16_t input, std::uint16_t mask) -> bool {
    auto es_mask = static_cast<std::uint16_t>(
        // low
        ((~(last_input & mask))) &
        // to high
        ((last_input & mask) ^ (input & mask)));
    return es_mask != 0;
}
auto FallingEdge(std::uint16_t last_input, std::uint16_t input, std::uint16_t mask) -> bool {
    auto es_mask = static_cast<std::uint16_t>(
        // high
        ((last_input & mask)) &
        // to low
        ((last_input & mask) ^ (input & mask)));
    return es_mask != 0;
}

void ReactToSwitch(std::uint16_t input,  // input
                   std::uint16_t last_input) {
    auto es_pressed = FallingEdge(last_input, input, kEMERGENCY_STOP);
    auto es_release = RisingEdge(last_input, input, kEMERGENCY_STOP);
    auto m10        = RisingEdge(last_input, input, kI_M10);

    auto switched     = static_cast<std::uint16_t>((last_input ^ input) & kI_SWITCH);
    auto switch_input = static_cast<std::uint16_t>(input & kI_SWITCH);

    if (es_pressed) {
        SendEvent(ESPressed());
    } else if (es_release) {
        SendEvent(ESReleased());
    }

    if (m10) {
        SendEvent(M10Rising());
    }

    if (switched) {
        if (switch_input == kI_SWITCH_AUTO &&
            BushingMachine::GetMode() != BushingMachine::Mode::Auto) {
            SendEvent(SwitchToAuto());
        } else if (switch_input == kI_SWITCH_MANUAL &&
                   BushingMachine::GetMode() != BushingMachine::Mode::Manual) {
            SendEvent(SwitchToManual());
        } else if (switch_input == kI_SWITCH_RESET &&
                   BushingMachine::GetMode() != BushingMachine::Mode::Reset) {
            SendEvent(SwitchToReset());
        }
    }
}

class Panic : public BushingMachine {
    void react(ESReleased const& /*e*/) override { transit<Initial>(); }
    void react(Operate const&) override {
        // do noting in panic mode
        return;
    }
};

class Initial : public BushingMachine {
    void entry() override { this->rising_edges = 0; }
    void react(Operate const& /*e*/) override {
        // do nothing in initial mode
        SendEvent(ResetCylinder());
        return;
    }
};

class Auto : public BushingMachine {
    void react(Operate const& e) override {
        auto operate_pressed = RisingEdge(last_input, e.input, kI_OPERATE);
        if (operate_pressed) {
            this->rising_edges++;
        }
        if (this->rising_edges % 2 == 1) {
            Transit next(e.input);
            SendEvent(next);
        }
    }
};

class Manual : public BushingMachine {
    void react(Operate const& e) override {
        auto operate_pressed = RisingEdge(last_input, e.input, kI_OPERATE);
        if (operate_pressed) {
            // std::cout << "operate" << std::endl;
            this->rising_edges++;
            Transit next(e.input);
            SendEvent(next);
        }
    }
};

class Reset : public BushingMachine {
    void react(Operate const& e) override {
        auto operate_pressed = RisingEdge(last_input, e.input, kI_OPERATE);
        if (operate_pressed) {
            this->rising_edges++;
            SendEvent(ResetCylinder());
        }
    }
};

void BushingMachine::entry() { this->rising_edges = 0; }

void BushingMachine::react(SwitchToAuto const&) {
    this->mode = Mode::Auto;
    transit<Auto>();
}
void BushingMachine::react(SwitchToManual const&) {
    this->mode = Mode::Manual;
    transit<Manual>();
}
void BushingMachine::react(SwitchToReset const&) {
    this->mode = Mode::Reset;
    transit<Reset>();
}
void BushingMachine::react(ESPressed const&) { transit<Panic>(); }
void BushingMachine::react(ESReleased const&) {}
void BushingMachine::react(M10Rising const&) { this->m10_rising = true; }

void BushingMachine::react(IOEvent const& e) {
    std::uint16_t input;
    memcpy(&input, &e.tx->i[0], sizeof(std::uint16_t));
    this->last_input = this->curr_input;
    this->curr_input = input;

    ReactToSwitch(input, this->last_input);
    Operate op(e.rx, input);
    SendEvent(op);
    Move p(e.rx);
    SendEvent(p);
}
void BushingMachine::react(Operate const& e) {}

std::uint16_t        BushingMachine::last_input   = kEMERGENCY_STOP;
std::uint16_t        BushingMachine::curr_input   = kEMERGENCY_STOP + kI_SWITCH;
std::uint64_t        BushingMachine::rising_edges = 0;
BushingMachine::Mode BushingMachine::mode         = BushingMachine::Mode::Initial;
bool                 BushingMachine::m10_rising   = false;

FSM_INITIAL_STATE(BushingMachine, Initial)
