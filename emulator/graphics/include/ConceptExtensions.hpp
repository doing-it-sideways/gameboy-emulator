#pragma once

#include <concepts>
#include <type_traits>
#include <glm/glm.hpp>

namespace cyber {

template <typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template <typename T, typename... Ts>
concept all_same = (std::is_same_v<T, Ts> && ...);

template <typename T, typename... Ts>
concept all_same_no_extent = (all_same<std::remove_extent_t<T>, std::remove_extent_t<Ts>...>);

template <typename>
struct is_glm_vec : std::false_type {};

template <typename T, glm::length_t L, glm::qualifier Q>
struct is_glm_vec<glm::vec<L, T, Q>> : std::true_type {
	static constexpr glm::length_t len = L;
	static constexpr glm::qualifier qual = Q;
};

template <typename T>
concept glm_vec = is_glm_vec<T>::value;

template <typename>
struct is_glm_mat : std::false_type {};

template <typename T, glm::length_t C, glm::length_t R, glm::qualifier Q>
struct is_glm_mat<glm::mat<C, R, T, Q>> : std::true_type {
	static constexpr glm::length_t cols = C;
	static constexpr glm::length_t rows = R;
	static constexpr glm::qualifier qual = Q;
};

template <typename T>
concept glm_mat = is_glm_mat<T>::value;

template <typename T>
concept std140_element = arithmetic<T> || glm_vec<T> || glm_mat<T>;

template <typename... Ts>
concept std140 = (std140_element<Ts> && ...);

}
