#include <print>
#include <string_view>

#include "Core.hpp"
#include "CPU.hpp"
#include "ROM.hpp"

#ifdef DEBUG
#include <array>
#include <charconv>
#include <filesystem>
#include <ranges>
#include <iostream>

static int TestMain(int argc, char** argv);
static int RunTests(bool step = false);
static inline int RunTest(std::size_t testNum, bool step = false);
static int RunTest(const std::filesystem::path& test, bool step = false);
#endif

int main(int argc, char** argv) {
	using namespace gb;

#if defined(DEBUG) && defined(TESTS)
	auto simulatedArgs = std::to_array<const char*>({ "", "21" });
	return TestMain(simulatedArgs.size(), const_cast<char**>(simulatedArgs.data()));
#endif // DEBUG
}

#ifdef DEBUG

static const std::filesystem::path testPath = TESTPATH;

using namespace std::string_view_literals;

static std::vector<std::filesystem::path> testRoms{};

static int TestMain(int argc, char** argv) {
	// add all test roms from directories into test rom list
	for (auto& file : std::filesystem::recursive_directory_iterator{ testPath }) {
		if (file.is_regular_file() && file.path().extension() == ".gb")
			testRoms.push_back(file.path());
	}

	if (argc < 2) {
		std::println(stderr, "To run a specific test, input either a file path or a number. Tests available:");
		std::println(stderr, "To step through the program, add \"step\" as the second argument.");

		for (const auto [i, test] : std::views::enumerate(testRoms)) {
			std::string testStr = test.string();
			testStr = testStr.substr(testStr.find("tests") + 6);

			std::println(stderr, "\t{:02}. {}", i + 1, testStr);
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

			if (argc >= 3) {
				std::string_view stepArg = argv[2];
				bool shouldStep = (stepArg.compare("step") == 0);

				RunTest(testPath, shouldStep);
			}
			else
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

		if (argc >= 3) {
			std::string_view stepArg = argv[2];
			bool shouldStep = (stepArg.compare("step") == 0);

			return RunTest(testNum, shouldStep);
		}
		else
			return RunTest(testNum);
	}
}

static int RunTests(bool step) {
	for (const auto [i, test] : std::views::enumerate(testRoms)) {
		std::string testStr = test.string();
		testStr = testStr.substr(testStr.find_last_of('/'));

		std::println("Test {}: {}", i, testStr);
		
		if (int res = RunTest(test); res != 0)
			return res;
	}
}

static inline int RunTest(std::size_t testNum, bool step) {
	return RunTest(testPath / testRoms[testNum], step);
}

static int RunTest(const std::filesystem::path& test, bool step) {
	using namespace gb;
	auto data = rom::Load(test);

	if (!data.has_value())
		return -1;

	cpu::Context ctx(std::move(data.value()));

	if (step) {
		std::println("\n-----Press any Key to Execute the next instruction-----\n");
		ctx.Start();

		while (std::cin.get()) {
			if (!ctx.Update())
				break;
		}
	}
	else
		ctx.Run();

	// TODO
	return 0;
}

#endif // DEBUG
