#include <chrono>
#include <thread>
#include <stdexcept>

#include "Emulator.hpp"
#include "ConstexprAdditions.hpp"

namespace gb {

static auto LoadRom(const std::filesystem::path& romPath) {
	auto data = rom::Load(romPath);

	if (!data.has_value())
		throw std::runtime_error{ "Invalid ROM data." };

	return data.value();
}

Emu::Emu(const std::filesystem::path& romPath)
	: _memory(std::move(LoadRom(romPath)))
	, _cpuCtx(_memory)
	, _ppuCtx(_memory)
{}

void Emu::Run() {
	_isRunning = true;

	while (_isRunning && !_screen.IsClosed()) {
		if (_isPaused) {
			// TODO: add delay
			continue;
		}

		if (!Update()) {
			debug::cexpr::println(stderr, "Something went wrong!");
			return;
		}

		_screen.Update();
	}
}

bool Emu::Update() {
	if (_isPaused)
		return true;

	if (!_cpuCtx.Update())
		return false;

	if (_screen.IsClosed())
		return false;
	
	return _screen.Update();
}

} // namespace gb
