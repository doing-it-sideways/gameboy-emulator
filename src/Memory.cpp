#include "Memory.hpp"

namespace gb::mem {

constexpr u16 interrupts = 0x0000;    // restart and interrupts
constexpr u16 headerStart = 0x0100;   // cartridge header
constexpr u16 rom0Start = 0x0150;     // 16 KiB ROM bank
constexpr u16 romNStart = 0x4000;     // 16 KiB ROM bank
constexpr u16 vramStart = 0x8000;     // 8 KiB VRAM
constexpr u16 ramExtStart = 0xA000;   // 8 KiB Cartridge ram (if present)
constexpr u16 ramWork0Start = 0xC000; // 
constexpr u16 ramWork1Start = 0xD000; // 
constexpr u16 unusable0 = 0xE000;     // Use of this area is prohibited.
constexpr u16 oamStart = 0xFE00;      // object attribute memory
constexpr u16 unusable1 = 0xFEA0;     // Use of this area is prohibited.
constexpr u16 ioStart = 0xFF00;       // io registers
constexpr u16 hramStart = 0xFF80;     // High RAM (Zero page)
constexpr u16 regIE = 0xFFFF;         // Interrupt enable register

} // namespace gb::mem
