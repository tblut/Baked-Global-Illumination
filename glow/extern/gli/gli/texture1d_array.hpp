/// @brief Include to use 1d array textures.
/// @file gli/texture1d_array.hpp

#pragma once

#include "texture1d.hpp"

namespace gli
{
	/// 1d array texture
	class texture1d_array : public texture
	{
	public:
		typedef extent1d extent_type;

	public:
		/// Create an empty texture 1D array
		texture1d_array();

		/// Create a texture1d_array and allocate a new storage
		explicit texture1d_array(
			format_type Format,
			extent_type const& Extent,
			size_type Layers,
			size_type Levels,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture1d_array and allocate a new storage with a complete mipmap chain
		explicit texture1d_array(
			format_type Format,
			extent_type const& Extent,
			size_type Layers,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture1d_array view with an existing storage
		explicit texture1d_array(
			texture const& Texture);

		/// Create a texture1d_array view with an existing storage
		explicit texture1d_array(
			texture const& Texture,
			format_type Format,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseFace, size_type MaxFace,
			size_type BaseLevel, size_type MaxLevel,
			swizzles_type const& Swizzles = swizzles_type(SWIZZLE_RED, SWIZZLE_GREEN, SWIZZLE_BLUE, SWIZZLE_ALPHA));

		/// Create a texture view, reference a subset of an exiting storage
		explicit texture1d_array(
			texture1d_array const& Texture,
			size_type BaseLayer, size_type MaxLayer,
			size_type BaseLevel, size_type MaxLevel);

		/// Create a view of the texture identified by Layer in the texture array
		texture1d operator[](size_type Layer) const;

		/// Return the width of a texture instance
		extent_type extent(size_type Level = 0) const;

		/// Fetch a texel from a texture. The texture format must be uncompressed.
		template <typename genType>
		genType load(extent_type const& TexelCoord, size_type Layer, size_type Level) const;

		/// Write a texel to a texture. The texture format must be uncompressed.
		template <typename genType>
		void store(extent_type const& TexelCoord, size_type Layer, size_type Level, genType const& Texel);

		/// Clear the entire texture storage with zeros
		void clear();

		/// Clear the entire texture storage with Texel which type must match the texture storage format block size
		/// If the type of genType doesn't match the type of the texture format, no conversion is performed and the data will be reinterpreted as if is was of the texture format. 
		template <typename genType>
		void clear(genType const& Texel);

		/// Clear a specific image of a texture.
		template <typename genType>
		void clear(size_type Layer, size_type Level, genType const& Texel);

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
		size_type index_cache(size_type Layer, size_type Level) const;

		std::vector<cache> Caches;
	};
}//namespace gli

#include "./core/texture1d_array.inl"
