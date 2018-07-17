/// @brief Include to compute offset in a texture storage memory.
/// @file gli/index.hpp

#pragma once

#include "type.hpp"

namespace gli
{
	/// Compute an offset from the beginning of a storage to a specific texel stored in linear order.
	template <typename T, precision P>
	inline size_t linear_index(tvec1<T, P> const & Coord, tvec1<T, P> const & Dimensions)
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Dimensions)));
		return static_cast<size_t>(Coord.x);
	}

	/// Compute an offset from the beginning of a storage to a specific texel stored in linear order.
	template <typename T, precision P>
	inline size_t linear_index(tvec2<T, P> const & Coord, tvec2<T, P> const & Dimensions)
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Dimensions)));
		return static_cast<size_t>(Coord.x + Coord.y * Dimensions.x);
	}

	/// Compute an offset from the beginning of a storage to a specific texel stored in linear order.
	template <typename T, precision P>
	inline size_t linear_index(tvec3<T, P> const & Coord, tvec3<T, P> const & Dimensions)
	{
		GLI_ASSERT(glm::all(glm::lessThan(Coord, Dimensions)));
		return static_cast<size_t>(Coord.x + Coord.y * Dimensions.x + Coord.z * Dimensions.x * Dimensions.y);
	}
}//namespace gli

