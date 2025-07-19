#pragma once

#include <filesystem>

#include "Core.hpp"
#include "HardwareRegisters.hpp"
#include "Memory.hpp"
#include "CPU.hpp"
#include "PPU.hpp"
#include "Screen.hpp"

namespace gb {

class Emu {
public:
	explicit Emu(const std::filesystem::path& romPath);

	// For stepping through instead of running
	void Start() { _isRunning = true; _isMultithreaded = false; }

	// Handles the update loop itself.
	void Run();

	// Update "loop".
	// Is public so the cpu can be easily stepped through from outside the class.
	[[nodiscard]] bool Update();

#if defined(DEBUG) && defined(TESTS)
	constexpr auto&& DebugMemory() noexcept { return _memory; }
	constexpr void SetDump(bool longDump, bool shortDump = false) noexcept { 
		_cpuCtx.longDump = longDump;
		_cpuCtx.shortDump = shortDump;
	}
#endif
private:
	using Clock = std::chrono::high_resolution_clock;
	using Time = Clock::time_point;

	using TargetSpeed = std::chrono::duration<double, std::ratio<1, 60>>;
	static constexpr TargetSpeed oneFrame = TargetSpeed{ 1 };

private:
	bool CoreUpdate();
	bool ScreenUpdate();

	bool ProcessCycles(u64 mCycles);
	void LimitSpeed();

private:
	Time _frameStart;

	Timer _timer;
	Memory _memory;

	cpu::Context _cpuCtx;
	ppu::GContext _ppuCtx;

	Screen _screen{};

	bool _isRunning = false;
	bool _isPaused = false;

	static inline bool _isMultithreaded = false;
};

} // namespace gb
