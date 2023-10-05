#pragma once

#include "bushing-machine.hpp"
#include "cylinder.hpp"
#include "tinyfsm.hpp"

using fsm_list = tinyfsm::FsmList<BushingMachine, Cylinders>;

template <typename E>
void SendEvent(E const& event) {
    fsm_list::template dispatch<E>(event);
}