#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "Core.hpp"

namespace gb::rom {

using RomData = std::vector<byte>;
std::optional<RomData> Load(const std::filesystem::path& romPath);

}

