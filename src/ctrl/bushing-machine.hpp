#pragma once

#include <cstdint>
#include <iostream>

#include "ec/pdo_def.h"
#include "tinyfsm.hpp"

using RX = icnc::ecat::RxPdo_t;
using TX = icnc::ecat::TxPdo_t;

struct IOEvent : tinyfsm::Event {
    RX* const       rx;
    const TX* const tx;

   public:
    IOEvent(RX* const o, const TX* const i) : rx(o), tx(i) {}
};

struct SwitchToAuto : tinyfsm::Event {};
struct SwitchToReset : tinyfsm::Event {};
struct SwitchToManual : tinyfsm::Event {};
struct M10Rising : tinyfsm::Event {};

struct Operate : tinyfsm::Event {
    std::uint16_t input;
    RX* const     rx;

   public:
    Operate(RX* const o, std::uint16_t i) : rx(o), input(i) {}
};

struct ESPressed : tinyfsm::Event {};
struct ESReleased : tinyfsm::Event {};

class BushingMachine : public tinyfsm::Fsm<BushingMachine> {
   public:
    enum class Mode { Initial, Reset, Manual, Auto };

   public:
    void react(tinyfsm::Event const&){};

    void react(SwitchToAuto const&);
    void react(SwitchToManual const&);
    void react(SwitchToReset const&);
    void react(ESPressed const&);
    void react(M10Rising const&);

    virtual void react(ESReleased const&);
    virtual void react(Operate const&);
    virtual void react(IOEvent const&);

    virtual void entry();  /* entry actions in some states */
    void         exit(){}; /* no exit actions at all */

    // virtual void react
   protected:
    static std::uint16_t last_input;
    static std::uint16_t curr_input;
    static std::uint64_t rising_edges;
    static Mode          mode;
    static bool          m10_rising;

   public:
    auto static GetMode() -> Mode const { return mode; }
    void static SetMode(Mode m) { mode = m; }
    auto static GetM10() -> bool { return m10_rising; }
    void static SetM10Down() { m10_rising = false; }
};
