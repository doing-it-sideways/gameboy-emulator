#pragma once

#include <cstddef>
#include <type_traits>
#include <glm/glm.hpp>

namespace cyber {

template <std::size_t N, typename T, typename... Ts>
struct nthType {
	using type = typename nthType<N - 1, Ts...>::type;
};

template <typename T, typename... Ts>
struct nthType<0, T, Ts...> {
	using type = T;
};

template <std::size_t N, typename... Ts>
using nthType_t = typename nthType<N, Ts...>::type;

template <typename... Ts>
using firstType_t = typename nthType<0, Ts...>::type;

template <typename... Ts>
using lastType_t = typename nthType<sizeof...(Ts) - 1, Ts...>::type;

} // namespace cyber
