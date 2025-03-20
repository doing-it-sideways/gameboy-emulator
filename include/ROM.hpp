#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "Core.hpp"

namespace gb::rom {

std::optional<std::vector<byte>> Load(const std::filesystem::path& romPath);

}

