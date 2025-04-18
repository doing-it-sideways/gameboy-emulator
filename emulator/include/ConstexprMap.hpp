#pragma once

#include <array>
#include <cstddef>
#include <utility>

namespace cyber {

template <typename Key, typename Value, std::size_t Size>
class ConstexprMap {
	using key_type = Key;
	using mapped_type = Value;
	using value_type = std::pair<const Key, Value>;
	using map_type = std::array<value_type, Size>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = const value_type&;
	using iterator = typename map_type::iterator;
	using const_iterator = typename map_type::const_iterator;
	using reverse_iterator = typename map_type::reverse_iterator;
	using const_reverse_iterator = typename map_type::const_reverse_iterator;

	constexpr Value at(const Key& key) const {

	}

	constexpr operator[](const Key& key) const {
		return at(key);
	}

private:
	const std::array<value_type, Size> data;
};

}