#include "cylinder.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <queue>

#include "ec/pdo_def.h"
#include "fsmlist.hpp"
#include "masks.h"

using RXA = std::array<std::uint8_t, icnc::ecat::kEcRxPdoSize>;

class P0;  // 左上角
class P0PartClipped;

class P1New;
class P1;          // 右上角
class P1ChunkOff;  // 右上角先松开夹爪
class P1PrePush;   // 右上角先退料
class P1Ext;       // 右下角

class P2;  // 左下角
class P2ChunkOn;

auto TurnOn(std::uint16_t& last_output, std::uint16_t mask) -> RX {
	RX rx;
	last_output = static_cast<std::uint16_t>(last_output | mask);
	memcpy(rx.o.data(), &last_output, sizeof(last_output));
	return rx;
};

auto TurnOff(std::uint16_t& last_output, std::uint16_t mask) -> RX {
	RX rx;
	last_output = static_cast<std::uint16_t>(last_output & (~mask));
	memcpy(rx.o.data(), &last_output, sizeof(last_output));
	return rx;
};

auto CheckMask(std::uint16_t input, std::uint16_t on_mask) -> bool {
	bool on = ((input & on_mask) == on_mask);
	return on;
}

class Init : public Cylinders {
	void entry() override {
		direction = false;
		// fill buffer to p1 cmds
		this->last_output = 0;
		RX rx{.o = {{0}}};
		for (int idx = 0; idx < 1200; idx++) {
			this->cmd_buffer.emplace(rx);
		}
	};
	void react(Transit const& /*e*/) override { transit<P1>(); }
};

class P0 : public Cylinders {
	void entry() override {
		// turn on C4
		for (int idx = 0; idx < 1420 * 8; idx++) {
			RX rx = TurnOn(this->last_output, kO_C4);
			this->cmd_buffer.emplace(rx);
		}

		for (int idx = 0; idx < 480 * 8; idx++) {
			RX rx = TurnOn(this->last_output, kO_C1);
			this->cmd_buffer.emplace(rx);
		}
	}

	void react(Transit const& e) override {
		if (this->cmd_buffer.empty()) {
			if (CheckMask(e.input, kI_C4_ON)) {
				transit<P0PartClipped>();
			}
		}
	}
};

class P0PartClipped : public Cylinders {
	void entry() override {
		RX rx = TurnOff(this->last_output, kO_C3);
		this->cmd_buffer.emplace(rx);
	}

	void react(Transit const& e) override {
		// 机床指令输入
		if (this->cmd_buffer.empty()) {
			if (BushingMachine::GetMode() == BushingMachine::Mode::Auto &&
					CheckMask(e.input, (kI_C3_OFF + kI_M10)) && BushingMachine::GetM10()) {
				BushingMachine::SetM10Down();
				if (CheckMask(e.input, kI_CHUNK_OFF)) {
					transit<P1PrePush>();
				} else {
					transit<P1ChunkOff>();  // 先松开夹爪
				}

			} else if (BushingMachine::GetMode() == BushingMachine::Mode::Manual) {
				if (CheckMask(e.input, kI_CHUNK_OFF)) {
					transit<P1PrePush>();
				} else {
					transit<P1ChunkOff>();  // 先松开夹爪
				}
			}
		}
	}
};

class P1ChunkOff : public Cylinders {
	void entry() override {
		RX rx;

		rx = TurnOff(this->last_output, kO_CHUCK);
		for (int idx = 0; idx < 750 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}

		rx = TurnOn(this->last_output, kO_CHUCK);
		for (int idx = 0; idx < 350 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}

		rx = TurnOff(this->last_output, kO_CHUCK);
		for (int idx = 0; idx < 500 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}
	}
	void react(const Transit& e) override {
		if (this->cmd_buffer.empty()) {
			if (CheckMask(e.input, kI_CHUNK_OFF)) {
				transit<P1PrePush>();
			} else {
				transit<P1ChunkOff>();
			}
		}
	}
};

class P1New : public Cylinders {
	void entry() override {
		// new product
		RX rx;
		// turn on machine
		rx = TurnOn(this->last_output, kO_SAFE);
		for (int idx = 0; idx < 200 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}
		// 回到原点, clear all output
		rx = {.o = {{0}}};
		for (int idx = 0; idx < 180; idx++) {
			this->cmd_buffer.emplace(rx);
		}
		this->last_output = 0;
		direction         = false;
	}
	void react(const Transit& /*e*/) override {
		if (this->cmd_buffer.empty()) {
			transit<P1>();
		}
	}
};

class P1PrePush : public Cylinders {
	void entry() override {
		RX rx = TurnOn(this->last_output, kO_C5);
		this->cmd_buffer.emplace(rx);

		direction = true;
	}
	void react(const Transit& e) override {
		if (this->cmd_buffer.empty()) {
			if (CheckMask(e.input, kI_C5_ON)) {
				transit<P1>();
			}
		}
	}
};

class P1 : public Cylinders {
	void entry() override {
		if (direction) {
			// from p1 to p3
			RX rx;
			// turn off C1,C5
			rx = TurnOff(this->last_output, kO_C1 + kO_C5);
			for (int idx = 0; idx < 180 * 8; idx++) {
				this->cmd_buffer.emplace(rx);
			}
			rx = TurnOn(this->last_output, kO_C2);
			this->cmd_buffer.emplace(rx);
		} else {
			// from p1 to p0
			RX rx;
			// turn on C3 伸出x轴
			rx = TurnOn(this->last_output, kO_C3);
			this->cmd_buffer.emplace(rx);
		}
	};

	void react(const Transit& e) override {
		// if guard fits
		// 确保X轴和Y轴已经到位
		if (this->cmd_buffer.empty()) {
			if (direction) {
				// 确保长推杆到底，退料气缸回退
				if (CheckMask(e.input, (kI_C2_ON + kI_C5_OFF))) {
					transit<P1Ext>();
				}
			} else {
				// 确保X轴已经到位
				if (CheckMask(e.input, kI_C3_ON)) {
					transit<P0>();
				}
			}
		}
	}
};

class P1Ext : public Cylinders {
	void entry() override {
		// turn on C3
		for (int idx = 0; idx < 450 * 8; idx++) {
			RX rx = TurnOn(this->last_output, kO_C3);
			this->cmd_buffer.emplace(rx);
		}
	}

	void react(const Transit& /*e*/) override {
		/*!
		 * 应该确保横推杆到位，但是没有传感器 用时延保证
		 */
		if (this->cmd_buffer.empty()) {
			transit<P2>();
		}
	}
};

class P2 : public Cylinders {
	void entry() override {
		RX rx;

		rx = TurnOn(this->last_output, kO_CHUCK);
		for (int idx = 0; idx < 300 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}

		rx = TurnOff(this->last_output, kO_CHUCK);
		for (int idx = 0; idx < 1250 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}
	};

	void react(Transit const& e) override {
		// if guard fits 卡盘卡紧
		if (this->cmd_buffer.empty()) {
			if (CheckMask(e.input, kI_CHUNK_ON)) {
				transit<P2ChunkOn>();
			} else if (CheckMask(e.input, kI_CHUNK_OFF)) {
				transit<P2>();
			}
		}
	}
};

class P2ChunkOn : public Cylinders {
	void entry() override {
		RX rx;

		memcpy(rx.o.data(),&last_output,sizeof(last_output));
		for (int idx=0 ;idx <200*8; idx++)
		{
			this->cmd_buffer.emplace(rx);
		}

		// turn off C4 张开夹爪
		rx = TurnOff(this->last_output, kO_C4);
		for (int idx = 0; idx < 30 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}
		// turn off C3 回缩
		rx = TurnOff(this->last_output, kO_C3);
		for (int idx = 0; idx < 220 * 8; idx++) {
			this->cmd_buffer.emplace(rx);
		}
		// turn off C2 回缩
		rx = TurnOff(this->last_output, kO_C2);
		this->cmd_buffer.emplace(rx);
	}

	void react(Transit const& e) override {
		if (this->cmd_buffer.empty()) {
			if (CheckMask(e.input, (kI_C3_OFF + kI_C4_OFF + kI_C2_OFF))) {
				transit<P1New>();
			}
		}
	}
};

void Cylinders::react(tinyfsm::Event const&) { return; }
void Cylinders::react(ResetCylinder const&) { transit<Init>(); }
void Cylinders::react(Move const& e) {
	if (!this->cmd_buffer.empty()) {
		e.rx->o = this->cmd_buffer.front().o;
		this->cmd_buffer.pop();
	}
}
void Cylinders::react(Transit const&) { return; }

std::queue<RX> Cylinders::cmd_buffer;
std::uint16_t  Cylinders::last_output = 0;
bool           Cylinders::direction   = false;

FSM_INITIAL_STATE(Cylinders, Init)
