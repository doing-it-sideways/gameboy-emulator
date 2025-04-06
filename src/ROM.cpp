#include <array>
#include <eternal.hpp>
#include <fstream>
#include <print>
#include <ranges>
#include <string_view>

#include "ROM.hpp"

namespace gb::rom {

static constexpr auto OldLicensees = mapbox::eternal::map<u8, std::string_view>(
{
#include "lists/OldLicensees.txt"	
});

// two character ascii licensee code
static constexpr auto NewLicensees = mapbox::eternal::map<std::string_view, std::string_view>(
{
#include "lists/NewLicensees.txt"
});

static constexpr auto CartridgeType = mapbox::eternal::map<u8, std::string_view>(
{
#include "lists/CartridgeMap.txt"
});

// Note: GBC only checks first 24 bytes, DMG/MGB check all bytes
static constexpr std::array<byte, 0x133 - 0x104 + 1> nintendoLogoBytes = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00,
	0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC,
	0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC,
	0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

std::optional<RomData> Load(const std::filesystem::path& romPath) {
	if (!std::filesystem::exists(romPath)) {
		std::println(stderr, "ROM at {} doesn't exist.", romPath.string());
		return std::nullopt;
	}

	if (!std::filesystem::is_regular_file(romPath)) {
		std::println(stderr, "A regular ROM file is needed.");
		return std::nullopt;
	}

	// from here, do the checksum to make sure it's actually a gb rom
	auto romSize = std::filesystem::file_size(romPath);

	if (romSize < 0x0150) // too small to even have a header
		return std::nullopt;

	if (romSize > (1 << 25)) // too big to be considered a gameboy rom no way
		return std::nullopt;

	// local anonymous struct babyyyyy
	struct {
		byte entryPoint[4];
		byte nintendoLogo[nintendoLogoBytes.size()];
		
		union {
			char title[16];
			
			struct {
				char title[0x013E - 0x0134 + 1]; // 11
				char mfc[4];
				byte cgbFlag;
			} newCartridgeShort;

			struct {
				char title[0x0142 - 0x0134 + 1]; // 15
				byte cgbFlag;
			} newCartridgeLong;
		};

		char newLicenseeCode[2];
		byte sgbFlag;
		u8 cartridgeType;
		u8 romSize;
		u8 ramSize;
		byte destCode;
		u8 oldLicenseeCode;
		byte romVersion;
		
		byte headerChecksum;

		union {
			u16 globalChecksum;
			byte globalChecksumBytes[2];
		};
	} *header;

	static_assert(sizeof(*header) == 0x014F - 0x0100 + 1);

	auto vec = std::make_optional<RomData>(romSize);

	{ // read in rom
		std::ifstream stream{ romPath, std::ios::binary };
		stream.read(reinterpret_cast<char*>(vec->data()), romSize);
		stream.close();
	}

	header = reinterpret_cast<decltype(header)>(vec->data() + 0x0100);

#pragma region debug printing for rom load
	std::println("----- Rom Loaded -----");
	// TODO: stop emulation when logo check fails
	std::println("\t- Nintendo Logo Check -- Matching? : {}",
				 std::ranges::equal(nintendoLogoBytes, header->nintendoLogo));

	// 0x33: new licensee code table should be used
	if (header->oldLicenseeCode == 0x33) {
		// if small title[last] != null, use big title
		if (header->newCartridgeShort.title[10] != '\0') {
			std::string_view title{ header->newCartridgeLong.title, 15 };
			std::println("\t- Title: {}", title);
			std::println("\t- Manufacturing Code: N/A");
			std::println("\t- Color Mode Support: {}", header->newCartridgeLong.cgbFlag == 0x80);
		}
		else {
			std::string_view title{ header->newCartridgeShort.title, 11 };
			std::println("\t- Title: {}", title);
			std::println("\t- Manufacturing Code: {}", header->newCartridgeShort.mfc);
			std::println("\t- Color Mode Support: {}", header->newCartridgeShort.cgbFlag == 0x80);
		}

		std::string_view licenseeCode{ header->newLicenseeCode, 2 };

		if (auto it = NewLicensees.find(licenseeCode); it != NewLicensees.end())
			std::println("\t- Publisher: {}", *it);
		else
			std::println("\t- Publisher: Unknown ({})", licenseeCode);
	}
	else {
		std::string_view title{ header->title, 16 }; // avoid overflows
		std::println("\t- Title: {}", title);

		std::println("\t- Manufacturing Code: N/A");
		std::println("\t- Color Mode Support: N/A");

		if (auto it = OldLicensees.find(header->oldLicenseeCode); it != OldLicensees.end())
			std::println("\t- Publisher: {}", *it);
		else
			std::println("\t- Publisher: Unknown ({:#04X})", header->oldLicenseeCode);
	}

	std::println("\t- SGB Flag: {:#04X}", header->sgbFlag);

	if (auto it = CartridgeType.find(header->cartridgeType); it != CartridgeType.end())
		std::println("\t- Cartridge Type: {}", it->second);
	else
		std::println("\t- Cartridge Type: Unknown ({:#04X})", header->cartridgeType);
	
	
	std::println("\t- ROM Size: {} KiB", 32 * (1 << header->romSize));
	
	switch (header->ramSize) {
	case 0x00:
		std::println("\t- RAM Size: N/A");
		break;
	case 0x02:
		std::println("\t- RAM Size: 8 KiB -- 1 bank");
		break;
	case 0x03:
		std::println("\t- RAM Size: 32 KiB -- 4 banks, 8 KiB each");
		break;
	case 0x04:
		std::println("\t- RAM Size: 128 KiB -- 16 banks, 8 KiB each");
		break;
	case 0x05:
		std::println("\t- RAM Size: 64 KiB -- 8 banks, 8 KiB each");
	}

	std::println("\t- Region Code: {}", (header->destCode == 0x00) ? "Japan+" : "Overseas");
	std::println("\t- ROM Version: {}", header->romVersion);
#pragma endregion

	// checksum -- taken right from gbdev.io
	u8 checksum = 0;
	for (u16 addr = 0x0134; addr <= 0x014C; ++addr) {
		checksum = checksum - vec.value()[addr] - 1;
	}

	if (u8 match = vec.value()[0x014D]; checksum != match) {
		std::println("\t- Checksum: {:#04X} (vs $014D) {:#04X}\n---FAILED---", checksum, match);
		return std::nullopt;
	}
	else
		std::println("\t- Checksum: {:#04X} (vs $014D) {:#04X}\n---SUCCESS---", checksum, match);

	return vec;
}

}
