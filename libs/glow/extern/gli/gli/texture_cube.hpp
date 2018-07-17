/// @brief Include to use cube map textures.
/// @file gli/texture_cube.hpp

#pragma once

#include "texture2d.hpp"

namespace gli
{
	/// Cube map texture
	class texture_cube : public texture
	{
	public:
		typedef extent2d extent_type;

	public:
		/// Create an empty texture cube
		texture_cube();

		/// Create a texture_cube and allocate a new storage
		explicit texture_cube(
			format_type Format,
			extent_type const & Extent,
			size_type Levels,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube and allocate a new storage with a complete mipmap chain
		explicit texture_cube(
			format_type Format,
			extent_type const & Extent,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube view with an existing storage
		explicit texture_cube(
			texture const & Texture);

		/// Create a texture_cube view with an existing storage
		explicit texture_cube(
			texture const & Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture_cube view, reference a subset of an existing texture_cube instance
		explicit texture_cube(
			texture_cube const & Texture,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the texture identified by Face in the texture cube
		texture2d operator[](size_type Face) const;

		/// Return the dimensions of a texture instance: width and height where both should be equal.
		extent_type extent(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename genType>
		genType load(extent_type const & TexelCoord, size_type Face, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename genType>
		void store(extent_type const & TexelCoord, size_type Face, size_type Level, genType const & Texel);

		/// Clear the entire texture storage with zeros
		void clear();

		/// Clear the entire texture storage with Texel which type must match the texture storage format block size
		/// If the type of genType doesn't match the type of the texture format, no conversion is performed and the data will be reinterpreted as if is was of the texture format. 
		template <typename genType>
		void clear(genType const & Texel);

		/// Clear a specific image of a texture.
		template <typename genType>
		void clear(size_type Face, size_type Level, genType const & Texel);

	private:
		struct cache
		{
			std::uint8_t* Data;
			extent_type Extent;
#			ifndef NDEBUG
				size_type Size;
#			endif
		};

		void build_cache();
		size_type index_cache(size_type Face, size_type Level) const;

		std::vector<cache> Caches;
	};
}//namespace gli

#include "./core/texture_cube.inl"
