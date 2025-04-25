#pragma once

#include <filesystem>

#include "Core.hpp"
#include "Memory.hpp"
#include "CPU.hpp"
#include "PPU.hpp"

namespace gb {

class Emu {
public:
	explicit Emu(const std::filesystem::path& romPath);

	// For stepping through instead of running
	constexpr void Start() { _isRunning = true; }

	// Handles the update loop itself.
	void Run();

	// Update "loop".
	// Is public so the cpu can be easily stepped through from outside the class.
	bool Update();

private:
	Memory _memory;

	cpu::Context _cpuCtx;
	ppu::GContext _ppuCtx;

	bool _isRunning = false;
	bool _isPaused = false;
};

} // namespace gb
