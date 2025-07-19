#include <chrono>
#include <stdexcept>
#include <thread>

#include "Emulator.hpp"
#include "ConstexprAdditions.hpp"
#include "DebugScreen.hpp"

namespace gb {

static auto LoadRom(const std::filesystem::path& romPath) {
	auto data = rom::Load(romPath);

	if (!data.has_value())
		throw std::runtime_error{ "Invalid ROM data." };

	return data.value();
}

Emu::Emu(const std::filesystem::path& romPath)
	: _timer()
	, _memory(std::move(LoadRom(romPath)), _timer)
	, _cpuCtx(_memory)
	, _ppuCtx(_memory)
{
	debug::InitDebugScreen(_screen.GetGLFWWindow(), &_memory);
}

void Emu::Run() {
	using namespace std::chrono_literals;
	
	_isRunning = true;
	_isMultithreaded = true;

	_frameStart = Clock::now();

	auto emuThread = std::jthread([this] {
		while (_isRunning) {
			if (_isPaused) {
				// TODO: better pause?
				std::this_thread::sleep_for(10ms);
				continue;
			}

			if (!CoreUpdate()) {
				debug::cexpr::println(stderr, "Something in the emulator went wrong!");

				_isRunning = false;
				return;
			}
		}
	});

	while (_isRunning) {
		if (!ScreenUpdate()) {
			_isRunning = false;
			debug::cexpr::println(stderr, "Something in the screen went wrong!");

			emuThread.join();
		}
	}
}

#ifdef DEBUG // TODO: REMOVE
#include <print>
static std::string debugStr{}, prevStr{};
#endif // DEBUG

bool Emu::CoreUpdate() {
	if (_isPaused)
		return true;

	if (!_cpuCtx.Update())
		return false;

	if (!ProcessCycles(_cpuCtx.GetUpdateCycles()))
		return false;

#ifdef DEBUG // TODO: REMOVE
	auto& mem = _memory;
	if (mem[0xFF02] == 0x81) {
		debugStr.push_back(static_cast<char>(mem[0xFF01]));
		mem[0xFF02] = 0;
	}

	if (debugStr != prevStr) {
		std::println(stderr, "{}", debugStr);
		prevStr = debugStr;
	}
#endif // DEBUG

	return true;
}

bool Emu::ScreenUpdate() {
	if (_screen.IsClosed())
		return false;

	return _screen.Update();
}

bool Emu::Update() {
	if (_isPaused)
		return true;

	if (!_cpuCtx.Update())
		return false;

	if (!ProcessCycles(_cpuCtx.GetUpdateCycles()))
		return false;

	if (_screen.IsClosed())
		return false;
	
	return _screen.Update();
}

bool Emu::ProcessCycles(u64 mCycles) {
	for (u64 mCycle = 0; mCycle < mCycles; ++mCycle) {
		for (u64 tCycle = 0; tCycle < 4; ++tCycle) {
			if (_timer.Tick()) {
				// TIMA overflows to 0, then one cycle later, IF is set
				//_cpuCtx.MCycle();
				_memory.GetInterruptFlag().flags.TimerInt = 1;
			}

			if (_ppuCtx.Update() == ppu::State::END_FRAME && _isMultithreaded)
				LimitSpeed();
		}

		if (_memory.IsDMAActive())
			_memory.DMATransferTick();
	}

	return true;
}

void Emu::LimitSpeed() {
	using namespace std::chrono_literals;

	Time frameEnd = Clock::now();
	TargetSpeed timeDiff = frameEnd - _frameStart;

	if (timeDiff < oneFrame)
		std::this_thread::sleep_for(oneFrame - timeDiff);

	_frameStart = Clock::now();
}

} // namespace gb
