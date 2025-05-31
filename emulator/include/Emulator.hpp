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
	void Start() { _isRunning = true; }

	// Handles the update loop itself.
	void Run();

	// Update "loop".
	// Is public so the cpu can be easily stepped through from outside the class.
	bool Update();

#if defined(DEBUG) && defined(TESTS)
	constexpr auto&& DebugMemory() noexcept { return _memory; }
	constexpr void SetDump(bool dump) noexcept { _cpuCtx.canDump = dump; }
#endif

private:
	void ProcessCycles(u64 mCycles);

private:
	Timer _timer;
	Memory _memory;

	cpu::Context _cpuCtx;
	ppu::GContext _ppuCtx;

	//Screen _screen{};

	bool _isRunning = false;
	bool _isPaused = false;
};

} // namespace gb
