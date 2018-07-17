#include "../levels.hpp"

namespace gli
{
	inline texture2d::texture2d()
	{}

	inline texture2d::texture2d(format_type Format, extent_type const& Extent, swizzles_type const& Swizzles)
		: texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, gli::levels(Extent), Swizzles)
	{
		this->build_cache();
	}

	inline texture2d::texture2d(format_type Format, extent_type const& Extent, size_type Levels, swizzles_type const& Swizzles)
		: texture(TARGET_2D, Format, texture::extent_type(Extent, 1), 1, 1, Levels, Swizzles)
	{
		this->build_cache();
	}

	inline texture2d::texture2d(texture const& Texture)
		: texture(Texture, TARGET_2D, Texture.format())
	{
		this->build_cache();
	}

	inline texture2d::texture2d
	(
		texture const& Texture,
		format_type Format,
		size_type BaseLayer, size_type MaxLayer,
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel,
		swizzles_type const& Swizzles
	)
		: texture(
			Texture, TARGET_2D, Format,
			BaseLayer, MaxLayer,
			BaseFace, MaxFace,
			BaseLevel, MaxLevel,
			Swizzles)
	{
		this->build_cache();
	}

	inline texture2d::texture2d
	(
		texture2d const& Texture,
		size_type BaseLevel, size_type MaxLevel
	)
		: texture(
			Texture, TARGET_2D, Texture.format(),
			Texture.base_layer(), Texture.max_layer(),
			Texture.base_face(), Texture.max_face(),
			Texture.base_level() + BaseLevel, Texture.base_level() + MaxLevel)
	{
		this->build_cache();
	}

	inline image texture2d::operator[](size_type Level) const
	{
		GLI_ASSERT(Level < this->levels());

		return image(
			this->Storage,
			this->format(),
			this->base_layer(),
			this->base_face(),
			this->base_level() + Level);
	}

	inline texture2d::extent_type texture2d::extent(size_type Level) const
	{
		GLI_ASSERT(!this->empty());

		return this->Caches[this->index_cache(Level)].Extent;
	}

	template <typename genType>
	inline genType texture2d::load(extent_type const& TexelCoord, size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(block_size(this->format()) == sizeof(genType));

		cache const & Cache = this->Caches[this->index_cache(Level)];

		std::size_t const Index = linear_index(TexelCoord, Cache.Extent);
		GLI_ASSERT(Index < Cache.Size / sizeof(genType));

		return reinterpret_cast<genType const* const>(Cache.Data)[Index];
	}

	template <typename genType>
	inline void texture2d::store(extent_type const& TexelCoord, size_type Level, genType const& Texel)
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(!is_compressed(this->format()));
		GLI_ASSERT(block_size(this->format()) == sizeof(genType));

		cache const & Cache = this->Caches[this->index_cache(Level)];
		GLI_ASSERT(glm::all(glm::lessThan(TexelCoord, Cache.Extent)));

		size_type const Index = linear_index(TexelCoord, Cache.Extent);
		GLI_ASSERT(Index < Cache.Size / sizeof(genType));

		reinterpret_cast<genType*>(Cache.Data)[Index] = Texel;
	}

	inline void texture2d::clear()
	{
		this->texture::clear();
	}

	template <typename genType>
	inline void texture2d::clear(genType const& Texel)
	{
		this->texture::clear<genType>(Texel);
	}

	template <typename genType>
	inline void texture2d::clear(size_type Level, genType const& Texel)
	{
		this->texture::clear<genType>(0, 0, Level, Texel);
	}

	inline texture2d::size_type texture2d::index_cache(size_type Level) const
	{
		return Level;
	}

	inline void texture2d::build_cache()
	{
		this->Caches.resize(this->levels());

		for(size_type LevelIndex = 0, LevelCount = this->levels(); LevelIndex < LevelCount; ++LevelIndex)
		{
			cache& Cache = this->Caches[this->index_cache(LevelIndex)];
			Cache.Data = this->data<std::uint8_t>(0, 0, LevelIndex);
			Cache.Extent = glm::max(extent_type(this->texture::extent(LevelIndex)), extent_type(1));
#			ifndef NDEBUG
				Cache.Size = this->size(LevelIndex);
#			endif
		}
	}
}//namespace gli
