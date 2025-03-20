#include <print>
#include <string_view>

#include "Core.hpp"
#include "ROM.hpp"

#ifdef DEBUG
#include <array>
#include <charconv>
#include <filesystem>
#include <ranges>

static int TestMain(int argc, char** argv);
static int RunTests();
static inline int RunTest(std::size_t testNum);
static int RunTest(const std::filesystem::path& test);
#endif

int main(int argc, char** argv) {
	using namespace gb;

#if defined(DEBUG) && defined(TESTS)
	return TestMain(argc, argv);
#endif // DEBUG

	rom::Load("F:/C++ Projects/gameboy emulator/roms/tetris.gb");
}

#ifdef DEBUG

static const std::filesystem::path romPath = ROMPATH;

using namespace std::string_view_literals;
static constexpr std::array testRoms {
	"cgb_sound/cgb_sound.gb"sv,
	"cpu_instrs/cpu_instrs.gb"sv,
	"dmg_sound/dmg_sound.gb"sv,
	"halt_bug/halt_bug.gb"sv,
	"instr_timing/instr_timing.gb"sv,
	"interrupt_time/interrupt_time.gb"sv,
	"mem_timing/mem_timing.gb"sv,
	"mem_timing-2/mem_timing-2.gb"sv,
	"oam_bug/oam_bug.gb"sv
};

static int TestMain(int argc, char** argv) {
	if (argc < 2) {
		std::println(stderr, "To run a specific test, input either a file path or a number. Tests available:");
		
		for (const auto [i, test] : std::views::enumerate(testRoms)) {
			std::println(stderr, "\t{}. {}", i + 1, (romPath / test).string());
		}

		return 1;
	}
	else {
		std::string_view str = argv[1];
		std::size_t testNum;
		auto [res, ec] = std::from_chars(str.data(), str.data() + str.size(), testNum);
		
		// filepath started with number, attempt to parse as filepath
		// this is also for if i forget to turn off test mode before launching a regular rom
		if (!std::string_view{ res }.empty()) {
			std::filesystem::path testPath = str;

			if (!std::filesystem::exists(testPath)) {
				std::println(stderr, "Not a valid filepath.");
				return 1;
			}

			return RunTest(testPath);
		}
		else if (ec == std::errc::invalid_argument) {
			std::println(stderr, "Not a valid test number.");
			return static_cast<int>(ec);
		}
		else if (ec == std::errc::result_out_of_range) {
			std::println(stderr, "Number is too big, there aren't that many tests.");
			return static_cast<int>(ec);
		}
		else if (testNum > testRoms.size()) {
			std::println(stderr, "Number is too big, there are only {} tests.", testRoms.size());
			return 1;
		}
		
		return RunTest(testNum + 1);
	}
}

static int RunTests() {
	for (const auto [i, testPath] : std::views::enumerate(testRoms)) {
		std::println("Test {}: {}", i, testPath.substr(testPath.find('/')));
		
		if (int res = RunTest(romPath / testPath); res != 0)
			return res;
	}
}

static inline int RunTest(std::size_t testNum) {
	return RunTest(romPath / testRoms[testNum]);
}

static int RunTest(const std::filesystem::path& test) {
	// TODO
	return 0;
}

#endif // DEBUG
