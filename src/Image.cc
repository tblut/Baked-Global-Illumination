#include "Image.hh"

#include <glow/common/log.hh>
#include <glow/data/SurfaceData.hh>
#include <glow/data/TextureData.hh>
#include <glow/objects/Texture2D.hh>
#include <glm/common.hpp>

#include <algorithm>
#include <cassert>

Image::Image(int width, int height, GLenum format)
		: width(width), height(height), format(format) {
	switch (format) {
	case GL_R8:
		channels = 1;
		bitsPerPixel = 8;
		break;

	case GL_R16F:
		channels = 1;
		bitsPerPixel = 16;
		break;

	case GL_RGB:
	case GL_RGB8:
	case GL_SRGB:
	case GL_SRGB8:
		channels = 3;
		bitsPerPixel = 24;
		break;

	case GL_RGBA:
	case GL_RGBA8:
	case GL_SRGB_ALPHA:
	case GL_SRGB8_ALPHA8:
		channels = 4;
		bitsPerPixel = 32;
		break;

	case GL_RGB16F:
		channels = 3;
		bitsPerPixel = 48;
		break;

	case GL_RGBA16F:
		channels = 4;
		bitsPerPixel = 64;
		break;

	case GL_RGB32F:
		channels = 3;
		bitsPerPixel = 96;
		break;

	case GL_RGBA32F:
		channels = 4;
		bitsPerPixel = 128;
		break;

	default:
		glow::error() << "Unsupported data type for image";
		break;
	}

	data.resize(width * height * bitsPerPixel / 8, 0);
}

int Image::getWidth() const {
	return width;
}

int Image::getHeight() const {
	return height;
}

int Image::getChannels() const {
	return channels;
}

int Image::getBitsPerPixel() const {
	return bitsPerPixel;
}

GLenum Image::getFormat() const {
    return format;
}

unsigned char* Image::getDataPtr() {
	return data.data();
}

const unsigned char* Image::getDataPtr() const {
	return data.data();
}

void Image::setWrapMode(GLenum wrapS, GLenum wrapT) {
	this->wrapS = wrapS;
	this->wrapT = wrapT;
}

GLenum Image::getWrapS() const {
	return wrapS;
}

GLenum Image::getWrapT() const {
	return wrapT;
}

glm::vec4 Image::sample(glm::vec2 uv) const {
	if (wrapS == GL_REPEAT) uv.x = static_cast<float>(std::fmod(uv.x, 1.0));
	if (wrapT == GL_REPEAT) uv.y = static_cast<float>(std::fmod(uv.y, 1.0));
	if (uv.x < 0.0f) uv.x += 1.0f;
	if (uv.y < 0.0f) uv.y += 1.0f;

	glm::ivec2 coord00 = glm::ivec2(uv.x * width, uv.y * height);
	glm::ivec2 coord10 = glm::ivec2(std::min(coord00.x + 1, width - 1), coord00.y);
	glm::ivec2 coord01 = glm::ivec2(coord00.x, std::min(coord00.y + 1, height - 1));
	glm::ivec2 coord11 = glm::ivec2(coord10.x, coord01.y);

	std::size_t index00 = (coord00.x + (height - coord00.y - 1) * width) * channels;
	std::size_t index10 = (coord10.x + (height - coord10.y - 1) * width) * channels;
	std::size_t index01 = (coord01.x + (height - coord01.y - 1) * width) * channels;
	std::size_t index11 = (coord11.x + (height - coord11.y - 1) * width) * channels;

	glm::vec4 color00(0.0f);
	glm::vec4 color10(0.0f);
	glm::vec4 color01(0.0f);
	glm::vec4 color11(0.0f);

    float scale = 1.0f / 255.0f;
    if (format == GL_RGB16F || format == GL_RGBA16F || format == GL_RGB32F || format == GL_RGBA32F) {
        scale = 1.0f;
    }
    
	for (int i = 0; i < channels; ++i) color00[i] = getDataPtr<unsigned char>()[index00 + i] * scale;
	for (int i = 0; i < channels; ++i) color10[i] = getDataPtr<unsigned char>()[index10 + i] * scale;
	for (int i = 0; i < channels; ++i) color01[i] = getDataPtr<unsigned char>()[index01 + i] * scale;
	for (int i = 0; i < channels; ++i) color11[i] = getDataPtr<unsigned char>()[index11 + i] * scale;

	float dx = uv.x * width - coord00.x;
	float dy = uv.y * height - coord00.y;

	return glm::mix(glm::mix(color00, color10, dx), glm::mix(color01, color11, dx), dy);
}


glow::SharedTexture2D Image::createTexture() const {
	auto surface = std::make_shared<glow::SurfaceData>();
	auto dataPtr = getDataPtr<char>();
	surface->setData(std::vector<char>(dataPtr, dataPtr + width * height * bitsPerPixel / 8));
	surface->setMipmapLevel(0);
	surface->setWidth(width);
	surface->setHeight(height);

	if (format == GL_R16F || format == GL_RGB16F || format == GL_RGBA16F) {
		surface->setType(GL_HALF_FLOAT);
	}
	else if (format == GL_R32F || format == GL_RGB32F || format == GL_RGBA32F) {
		surface->setType(GL_FLOAT);
	}
	else {
		surface->setType(GL_UNSIGNED_BYTE);
	}

	auto tex = std::make_shared<glow::TextureData>();
	tex->setWidth(width);
	tex->setHeight(height);
	tex->addSurface(surface);

	if (channels == 1) {
		tex->setPreferredInternalFormat(format);
		surface->setFormat(GL_RED);
	}
	else if (channels == 3) {
		tex->setPreferredInternalFormat(format);
		surface->setFormat(GL_RGB);
	}
	else if (channels == 4) {
		tex->setPreferredInternalFormat(format);
		surface->setFormat(GL_RGBA);
	}
	else {
		glow::error() << "Unsupported image component count";
	}

	tex->setAnisotropicFiltering(16.0f);
	tex->setMagFilter(GL_LINEAR);
	tex->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	tex->setWrapS(wrapS);
	tex->setWrapT(wrapT);

	auto tex2D = glow::Texture2D::createFromData(tex);
	tex2D->bind().generateMipmaps();
	return tex2D;
}
