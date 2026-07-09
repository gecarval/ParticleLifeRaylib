#ifndef VECTOR2I_HPP
#define VECTOR2I_HPP

#include "../include/raylib-cpp.hpp"

struct Vector2i {
	int x;
	int y;
	constexpr Vector2i(int x = 0, int y = 0) noexcept : x(x), y(y) {
	}
	constexpr Vector2i(const Vector2i &v) noexcept : x(v.x), y(v.y) {
	}
	constexpr Vector2i(const raylib::Vector2 &v) noexcept
		: x(static_cast<int>(v.x)), y(static_cast<int>(v.y)) {
	}
	constexpr Vector2i &operator=(const Vector2i &other) noexcept {
		if (this != &other) {
			this->x = other.x;
			this->y = other.y;
		}
		return (*this);
	}
	constexpr bool operator==(const Vector2i &other) const noexcept {
		return x == other.x && y == other.y;
	}
	constexpr bool operator!=(const Vector2i &other) const noexcept {
		return x != other.x || y != other.y;
	}
};

struct Vector2iHash {
	inline std::size_t operator()(const Vector2i &v) const noexcept {
		std::size_t h = static_cast<std::size_t>(v.x);
		h ^= static_cast<std::size_t>(v.y) + 0x9e3779b9u + (h << 6) + (h >> 2);
		return h;
	}
};

#endif
