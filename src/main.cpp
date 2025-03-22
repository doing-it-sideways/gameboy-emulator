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
	for (const auto [i, test] : std::views::enumerate(testRoms)) {
		std::string testStr = test.string();
		testStr = testStr.substr(testStr.find_last_of('/'));

		std::println("Test {}: {}", i, testStr);
		
		if (int res = RunTest(test); res != 0)
			return res;
	}
}

static inline int RunTest(std::size_t testNum) {
	return RunTest(testPath / testRoms[testNum]);
}

static int RunTest(const std::filesystem::path& test) {
	using namespace gb;
	auto data = rom::Load(test);

	if (!data.has_value())
		return -1;

	// TODO
	return 0;
}

#endif // DEBUG
