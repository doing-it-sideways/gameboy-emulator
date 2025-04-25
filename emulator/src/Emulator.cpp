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

	while (_isRunning) {
		if (_isPaused) {
			// TODO: add delay
			continue;
		}

		if (!Update()) {
			debug::cexpr::println(stderr, "Something went wrong!");
			return;
		}
	}
}

bool Emu::Update() {
	if (!_cpuCtx.Update())
		return false;

	return true;
}

} // namespace gb
