namespace gb {

static constexpr u16 interruptEnd = 0x0100;	// restarts and interrupts [$0000, $0FFF]
static constexpr u16 headerEnd = 0x0150;	// cartridge header [$0100, $014F]
static constexpr u16 rom0End = 0x4000;		// fixed 16 KiB ROM bank [$0150, $3FFF]
static constexpr u16 romNEnd = 0x8000;		// switchable 16 KiB ROM bank [$4000, $7FFF]
static constexpr u16 vramEnd = 0xA000;		// 8 KiB VRAM [$8000, $9FFF]
static constexpr u16 ramExtEnd = 0xC000;	// 8 KiB Cartridge ram [$A000, $BFFF]
static constexpr u16 ramWork0End = 0xD000;	// [$C000, $CFFF]
static constexpr u16 ramWork1End = 0xE000;	// [$D000, $DFFF]
static constexpr u16 unusable0End = 0xFE00;	// Prohibited to use. [$E000, $FDFF]
static constexpr u16 oamEnd = 0xFEA0;		// object attribute memory [$FE00, $FE9F]
static constexpr u16 unusable1End = 0xFF00;	// Prohibited to use. [$FEA0, $FEFF]
static constexpr u16 ioEnd = 0xFF80;		// io registers [$FF00, $FF7F]
static constexpr u16 hramEnd = 0xFFFF;		// High RAM (Zero page) [$FF80, $FFFE]
static constexpr u16 regIE = 0xFFFF;		// Interrupt enable register ($FFFF)

template <typename Self>
auto Memory::operator[](this Self&& self, u16 addr) {
	if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
		byte val = const_cast<Memory&>(self).Read(addr);
		return val;
	}
	else
		return Accessor{ self, addr };
}

} // namespace gb

