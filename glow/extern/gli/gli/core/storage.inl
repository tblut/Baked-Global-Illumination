namespace gli
{
	inline storage::storage()
		: Layers(0)
		, Faces(0)
		, Levels(0)
		, BlockSize(0)
		, BlockCount(0)
		, BlockExtent(0)
		, Extent(0)
	{}

	inline storage::storage(format_type Format, extent_type const & Extent, size_type Layers, size_type Faces, size_type Levels)
		: Layers(Layers)
		, Faces(Faces)
		, Levels(Levels)
		, BlockSize(gli::block_size(Format))
		, BlockCount(glm::max(Extent / gli::block_extent(Format), extent_type(1)))
		, BlockExtent(gli::block_extent(Format))
		, Extent(Extent)
	{
		GLI_ASSERT(Layers > 0);
		GLI_ASSERT(Faces > 0);
		GLI_ASSERT(Levels > 0);
		GLI_ASSERT(glm::all(glm::greaterThan(Extent, extent_type(0))));

		this->Data.resize(this->layer_size(0, Faces - 1, 0, Levels - 1) * Layers, 0);
	}

	inline bool storage::empty() const
	{
		return this->Data.empty();
	}

	inline storage::size_type storage::layers() const
	{
		return this->Layers;
	}

	inline storage::size_type storage::faces() const
	{
		return this->Faces;
	}

	inline storage::size_type storage::levels() const
	{
		return this->Levels;
	}

	inline storage::size_type storage::block_size() const
	{
		return this->BlockSize;
	}

	inline storage::extent_type storage::block_extent() const
	{
		return this->BlockExtent;
	}

	inline storage::extent_type storage::block_count(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->Levels);

		return glm::max(this->BlockCount >> storage::extent_type(static_cast<storage::extent_type::value_type>(Level)), storage::extent_type(1));
	}

	inline storage::extent_type storage::extent(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->Levels);

		return glm::max(this->Extent >> storage::extent_type(static_cast<storage::extent_type::value_type>(Level)), storage::extent_type(1));
	}

	inline storage::size_type storage::size() const
	{
		GLI_ASSERT(!this->empty());

		return static_cast<size_type>(this->Data.size());
	}

	inline storage::data_type * storage::data()
	{
		GLI_ASSERT(!this->empty());

		return &this->Data[0];
	}

	inline storage::size_type storage::base_offset(size_type Layer, size_type Face, size_type Level) const
	{
		GLI_ASSERT(!this->empty());
		GLI_ASSERT(Layer >= 0 && Layer < this->layers() && Face >= 0 && Face < this->faces() && Level >= 0 && Level < this->levels());

		size_type const LayerSize = this->layer_size(0, this->faces() - 1, 0, this->levels() - 1);
		size_type const FaceSize = this->face_size(0, this->levels() - 1);
		size_type BaseOffset = LayerSize * Layer + FaceSize * Face;

		for(size_type LevelIndex = 0, LevelCount = Level; LevelIndex < LevelCount; ++LevelIndex)
			BaseOffset += this->level_size(LevelIndex);

		return BaseOffset;
	}

	inline storage::size_type storage::level_size(size_type Level) const
	{
		GLI_ASSERT(Level >= 0 && Level < this->levels());

		return this->BlockSize * glm::compMul(this->block_count(Level));
	}

	inline storage::size_type storage::face_size(size_type BaseLevel, size_type MaxLevel) const
	{
		GLI_ASSERT(MaxLevel >= 0 && MaxLevel < this->levels());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < this->levels());
		GLI_ASSERT(BaseLevel <= MaxLevel);

		size_type FaceSize(0);

		// The size of a face is the sum of the size of each level.
		for(storage::size_type Level(BaseLevel); Level <= MaxLevel; ++Level)
			FaceSize += this->level_size(Level);

		return FaceSize;
	}

	inline storage::size_type storage::layer_size(
		size_type BaseFace, size_type MaxFace,
		size_type BaseLevel, size_type MaxLevel) const
	{
		GLI_ASSERT(BaseFace >= 0 && MaxFace < this->faces());
		GLI_ASSERT(BaseFace >= 0 && BaseFace < this->faces());
		GLI_ASSERT(MaxLevel >= 0 && MaxLevel < this->levels());
		GLI_ASSERT(BaseLevel >= 0 && BaseLevel < this->levels());

		// The size of a layer is the sum of the size of each face.
		// All the faces have the same size.
		return this->face_size(BaseLevel, MaxLevel) * (MaxFace - BaseFace + 1);
	}
}//namespace gli
