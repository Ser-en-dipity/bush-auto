#pragma once

#include <cstdint>
#include <queue>
#include <vector>

#include "ec/pdo_def.h"
#include "tinyfsm.hpp"

using TX = icnc::ecat::TxPdo_t;
using RX = icnc::ecat::RxPdo_t;

struct ResetCylinder : tinyfsm::Event {};

struct Transit : tinyfsm::Event {
    std::uint16_t input;

   public:
    explicit Transit(std::uint16_t i) : input(i) {}
};

struct Move : tinyfsm::Event {
    RX* const rx;

   public:
    explicit Move(RX* const o) : rx(o) {}
};

class Cylinders : public tinyfsm::Fsm<Cylinders> {
   public:
    void         react(tinyfsm::Event const&);
    void         react(ResetCylinder const&);
    void         react(Move const&);
    virtual void react(Transit const&);

    virtual void entry(){};
    void         exit(){};

   protected:
    static bool           direction;  // true clock_wise
    static std::uint16_t  last_output;
    static std::queue<RX> cmd_buffer;
    static bool           m10_rise;
};
