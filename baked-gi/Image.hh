#pragma once

#include <glow/gl.hh>
#include <glow/fwd.hh>
#include <glow/data/ColorSpace.hh>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

class Image {
public:
    Image(int width, int height, GLenum format = GL_SRGB);

	int getWidth() const;
	int getHeight() const;
	int getChannels() const;
	int getBitsPerPixel() const;
    GLenum getFormat() const;
	unsigned char* getDataPtr();
	const unsigned char* getDataPtr() const;

	void setWrapMode(GLenum wrapS, GLenum wrapT);
	GLenum getWrapS() const;
	GLenum getWrapT() const;

	template <typename T>
	T* getDataPtr() {
		return reinterpret_cast<T*>(data.data());
	}

	template <typename T>
	const T* getDataPtr() const {
		return reinterpret_cast<const T*>(data.data());
	}

	template <typename T>
	void setPixel(glm::uvec2 coord, T value) {
		getDataPtr<T>()[coord.x + coord.y * width] = value;
	}

	template <typename T>
	T getPixel(glm::uvec2 coord) const {
		return getDataPtr<T>()[coord.x + coord.y * width];
	}

	glm::vec4 sample(glm::vec2 uv) const;

	glow::SharedTexture2D createTexture() const;

private:
	int width;
	int height;
	int channels;
	int bitsPerPixel;
	GLenum format;
	std::vector<unsigned char> data;
	GLenum wrapS = GL_REPEAT;
	GLenum wrapT = GL_REPEAT;
};

using SharedImage = std::shared_ptr<Image>;
