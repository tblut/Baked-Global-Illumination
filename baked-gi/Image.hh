#pragma once

#include <glow/gl.hh>
#include <glow/fwd.hh>
#include <glow/data/ColorSpace.hh>
#include <vector>
#include <memory>

class Image {
public:
	Image(int width, int height, int channels, glow::ColorSpace colorSpace = glow::ColorSpace::sRGB);

	int getWidth() const;
	int getHeight() const;
	int getChannels() const;
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

	glow::SharedTexture2D createTexture() const;

private:
	int width;
	int height;
	int channels;
	std::vector<unsigned char> data;
	GLenum wrapS = GL_REPEAT;
	GLenum wrapT = GL_REPEAT;
	glow::ColorSpace colorSpace;
};

using SharedImage = std::shared_ptr<Image>;