#pragma once

#define DEFAULT_COPY(Type, DefaultVal) \
	constexpr Type(const Type&) = DefaultVal; \
	constexpr Type& operator=(const Type&) = DefaultVal

#define DEFAULT_MOVE(Type, DefaultVal) \
	constexpr Type(Type&&) = DefaultVal; \
	constexpr Type& operator=(Type&&) = DefaultVal
