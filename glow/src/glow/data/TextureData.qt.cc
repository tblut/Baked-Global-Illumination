#include "TextureData.hh"

#include <cassert>

#include "SurfaceData.hh"

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>


#ifdef GLOW_USE_QT
#include <QImage>

using namespace glow;

SharedTextureData TextureData::loadWithQt(const std::string &filename, const std::string &ending, ColorSpace colorSpace)
{
    GLOW_ACTION();

    QImage img;
    if (!img.load(QString::fromStdString(filename)))
    {
        error() << "Error loading QT texture data from " << filename;
        error() << "  Reason: unable to QImage::load.";
        return nullptr;
    }

    auto width = img.width();
    auto height = img.height();

    auto tex = std::make_shared<TextureData>();
    auto surface = std::make_shared<SurfaceData>();

    surface->setType(GL_UNSIGNED_BYTE);
    surface->setMipmapLevel(0);
    surface->setWidth(width);
    surface->setHeight(height);

    // switch (img.format())
    // {
    // default:
    //     img = img.convertToFormat(QImage::Format_ARGB32);
    // // fall-through on purpose!
    // case QImage::Format_ARGB32:
    //
    //     break;
    // }

    // For now: only ARGB
    img = img.convertToFormat(QImage::Format_ARGB32);
    std::vector<char> data;
    data.resize(img.byteCount());
    std::copy(img.bits(), img.bits() + img.byteCount(), begin(data));
    surface->setData(data);
    surface->setFormat(GL_RGBA);

    switch (colorSpace)
    {
    case ColorSpace::Linear:
        tex->setPreferredInternalFormat(GL_RGBA);
        break;
    case ColorSpace::sRGB:
    case ColorSpace::AutoDetect:
        tex->setPreferredInternalFormat(GL_SRGB_ALPHA);
        break;
    }

    tex->setWidth(width);
    tex->setHeight(height);
    tex->addSurface(surface);

	// high-quality default parameters:
	tex->setAnisotropicFiltering(16.0f);
	tex->setMagFilter(GL_LINEAR);
	tex->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);

    return tex;
}

void TextureData::saveWithQt(TextureData *data, const std::string &filename, const std::string &ending)
{
    GLOW_ACTION();

    for (auto const &surf : data->getSurfaces())
    {
        if (surf->getMipmapLevel() == 0)
        {
            if (surf->getType() != GL_UNSIGNED_BYTE)
            {
                error() << "Error saving QT texture data to " << filename;
                error() << "  Reason: type " << surf->getType() << " is not supported with QT (yet).";
                return;
            }

            // int comps = -1;
            SharedSurfaceData tmpS = surf;

            /*switch (surf->getFormat())
            {
            case GL_RGBA:
                comps = 4;
                break;

            case GL_BGRA:
                comps = 4;
                tmpS = surf->convertTo(GL_RGBA, surf->getType());
                break;

            case GL_RGB:
                comps = 3;
                break;

            case GL_RED:
            case GL_RG:
            case GL_BGR:
                comps = 3;
                tmpS = surf->convertTo(GL_RGB, surf->getType());
                break;

            default:
                error() << "Error saving QT texture data to " << filename;
                error() << "  Reason: format " << surf->getFormat() << " is not supported with QT (yet).";
                break;
            }*/
            // QT only properly supports 32bit colors
            if (surf->getFormat() != GL_BGRA)
                tmpS = surf->convertTo(GL_BGRA, surf->getType());

            QImage img((const uchar *)tmpS->getData().data(), tmpS->getWidth(), tmpS->getHeight(), QImage::Format_ARGB32);
            if (!img.save(QString::fromStdString(filename)))
            {
                error() << "Error saving QT texture data to " << filename;
                error() << "  Reason: QImage::save returned false.";
            }

            return;
        }
    }

    error() << "Error saving QT texture data to " << filename;
    error() << "  Reason: No supported level 0 surface found.";
}
#endif
