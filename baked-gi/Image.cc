#include "Image.hh"

#include <glow/common/log.hh>
#include <glow/data/SurfaceData.hh>
#include <glow/data/TextureData.hh>
#include <glow/objects/Texture2D.hh>
#include <glm/common.hpp>

#include <algorithm>
#include <cassert>

Image::Image(int width, int height, int channels, glow::ColorSpace colorSpace)
		: width(width), height(height), channels(channels),
		  data(width * height * channels), dataType(GL_UNSIGNED_BYTE), colorSpace(colorSpace) {

}

Image::Image(int width, int height, int channels, GLenum dataType, glow::ColorSpace colorSpace)
		: width(width), height(height), channels(channels),
		  dataType(dataType), colorSpace(colorSpace) {
    assert(!(dataType == GL_FLOAT && colorSpace == glow::ColorSpace::sRGB));
    
    if (dataType == GL_UNSIGNED_BYTE) {
        data.resize(width * height * channels);
    }
    else if (dataType == GL_FLOAT) {
        data.resize(width * height * channels * 4);
    }
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

GLenum Image::getDataType() const {
    return dataType;
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
	if (wrapS == GL_REPEAT) std::fmod(uv.x, 1.0);
	if (wrapT == GL_REPEAT) std::fmod(uv.y, 1.0);
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
    if (dataType == GL_FLOAT) {
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
    int channelStride = (dataType == GL_FLOAT) ? 4 : 1;
	surface->setData(std::vector<char>(dataPtr, dataPtr + width * height * channels * channelStride));
	surface->setType(dataType);
	surface->setMipmapLevel(0);
	surface->setWidth(width);
	surface->setHeight(height);

	auto tex = std::make_shared<glow::TextureData>();
	tex->setWidth(width);
	tex->setHeight(height);
	tex->addSurface(surface);

	GLenum iformatRGB;
	GLenum iformatRGBA;

    if (dataType == GL_UNSIGNED_BYTE) {
        switch (colorSpace)
        {
        case glow::ColorSpace::Linear:
            iformatRGB = GL_RGB;
            iformatRGBA = GL_RGBA;
            break;
        case glow::ColorSpace::sRGB:
        default:
            iformatRGB = GL_SRGB;
            iformatRGBA = GL_SRGB_ALPHA;
            break;
        }
    }
    else if (dataType == GL_FLOAT) {
        iformatRGB = GL_RGB32F;
        iformatRGBA = GL_RGBA32F;
    }
    else {
        glow::error() << "Unsupported data type for image";
    }

	if (channels == 1) {
		surface->setFormat(GL_RGB);
		tex->setPreferredInternalFormat(iformatRGB);

		// convert grey to rgb
		std::vector<char> newData;
		const auto& oldData = surface->getData();
		auto oldSize = oldData.size();
		newData.resize(oldData.size() * 3);
		for (std::size_t i = 0; i < oldSize; ++i)
		{
			auto d = oldData[i];
			newData[i * 3 + 0] = d;
			newData[i * 3 + 1] = d;
			newData[i * 3 + 2] = d;
		}
		surface->setData(newData);
	}
	else if (channels == 3) {
		tex->setPreferredInternalFormat(iformatRGB);
		surface->setFormat(GL_RGB);
	}
	else if (channels == 4) {
		tex->setPreferredInternalFormat(iformatRGBA);
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
