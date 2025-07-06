#include "PPU.hpp"
#include "Memory.hpp"

namespace gb::ppu {

GContext::GContext(Memory& memory)
	: _memory(memory)
{
	SetMode(Mode::OAM_SCAN);
}

bool GContext::Update() {
	// ly should only ever be updated inside the ppu
	byte& ly = _memory.Read(0xFF44);

	switch (GetMode()) {
	case Mode::HBLANK:
		if (_curDot == dotsPerLine) {
			_curDot = 0;

			if (ly == vBlankStartNext)
				SetMode(Mode::VBLANK);

			UpdateLine(ly);
			break;
		}

		break;
	case Mode::VBLANK:
		break;
	case Mode::OAM_SCAN:
		if (_curDot == oamScanDots)
			SetMode(Mode::PIXEL_DRAW);
		
		++_curDot;
		break;
	case Mode::PIXEL_DRAW:
		break;
	}

	return true;
}

void GContext::UpdateLine(byte& ly) {
	++ly;

	byte lycEqLy = static_cast<byte>(ly == _memory.Read(0xFF45));
	byte& stat = _memory.Read(0xFF41);

	// LCDStatus::LycEqLy should only be set here
	if (lycEqLy != 0) {
		LCDStatus statFlags = static_cast<LCDStatus>(stat);
		statFlags.flags.LycEqLy = lycEqLy;

		if (statFlags.flags.LycIntSelect == 1)
			_memory.GetInterruptFlag().flags.LCDInt = 1;

		stat = statFlags;
	}
	else
		stat &= ~(1 << 2); // LycEqLy = 0
}

Mode GContext::GetMode() const {
	return static_cast<Mode>(static_cast<LCDStatus>(_memory[0xFF41]).flags.PPUMode);
}

void GContext::SetMode(Mode newMode) {
	byte& stat = _memory.Read(0xFF41);
	
	stat &= ~3; // clear bits first
	stat |= static_cast<byte>(newMode); // then set to new value
}

PaletteData GContext::GetPaletteData(Palette palette) const {
	return static_cast<PaletteData>(_memory[0xFF47 + static_cast<byte>(palette)]);
}

void GContext::SetPaletteData(Palette palette, PaletteData newIndices) {
	_memory[0xFF47 + static_cast<byte>(palette)] = newIndices;
}

} // namespace gb::ppu
