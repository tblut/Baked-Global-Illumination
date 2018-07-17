#pragma once

#include <string>

#include <glow/common/macro_join.hh>
#include <glow/common/non_copyable.hh>
#include <glow/gl.hh>

#include <initializer_list>

namespace glow
{
/**
 * Various scope-guards for OpenGL state access
 *
 * And the end of their lifespan (i.e. scope), these objects restore the state when they were constructed
 *
 * Due to C++ syntax, you have to provide a name for these objects, even if you never access them
 *
 * Alternatively you can use the macro syntax:
 * GLOW_SCOPED(function, values...)
 * e.g.
 *   GLOW_SCOPED(enable, GL_BLEND);
 *   GLOW_SCOPED(clearColor, 0.f, 0.f, 0.f, 0.f);
 *
 * Usage/Doc:
 *
 *   scoped::disable        name(GL_DEPTH_TEST);
 *   scoped::enable         name(GL_BLEND);
 *   scoped::cullFace       name(GL_BACK);
 *   scoped::frontFace      name(GL_CCW);
 *   scoped::blendEquation  name(GL_FUNC_ADD);
 *   scoped::depthMask      name(true);
 *   scoped::depthFunc      name(GL_LESS);
 *   scoped::clearDepth     name(1.f);
 *   scoped::lineWidth      name(1.f);
 *   scoped::pointSize      name(1.f);
 *   scoped::blendFunc      name(GL_ONE, GL_ZERO);
 *   scoped::blendFunc      name(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO); // rgb, alpha
 *   scoped::clearColor     name(0.f, 0.f, 0.f, 0.f);
 *   scoped::colorMask      name(false, false, false, false);
 *   scoped::blendColor     name(0.f, 0.f, 0.f, 0.f);
 *   scoped::sampleCoverage name(1.f, GL_FALSE);
 *   scoped::polygonOffset  name(0.f, 0.f);
 *   scoped::pixelStore     name(GL_PACK_ROW_LENGTH, 0);
 *   scoped::viewport       name(0, 0, 800, 600);
 *   scoped::debugGroup     name(0, "test");
 *   scoped::polygonMode    name(GL_FRONT_AND_BACK, GL_FILL);
 *
 * Abbreviated pixel stores:
 *   scoped::(un)packSwapBytes   name(false);
 *   scoped::(un)packLsbFirst    name(false);
 *   scoped::(un)packRowLength   name(0);
 *   scoped::(un)packImageHeight name(0);
 *   scoped::(un)packSkipRows    name(0);
 *   scoped::(un)packSkipPixels  name(0);
 *   scoped::(un)packSkipImages  name(0);
 *   scoped::(un)packAlignment   name(4);
 *
 * TODO: Stencil func/op
 */

#define GLOW_SCOPED(funcName, ...) \
    glow::scoped::funcName GLOW_MACRO_JOIN(_glow_scoped_gl_, __COUNTER__)(__VA_ARGS__) // enforce ;

namespace scoped
{
/// Disables the given state
/// restores original value at the end
struct disable
{
    GLOW_NON_COPYABLE(disable);

    GLenum state;
    GLboolean prevState;

    disable(GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        glDisable(state);
    }
    ~disable()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

/// Enables the given state
/// restores original value at the end
struct enable
{
    GLOW_NON_COPYABLE(enable);

    GLenum state;
    GLboolean prevState;

    enable(GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        glEnable(state);
    }
    ~enable()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

/// Disables the given state if condition is true (otherwise keeps the same)
/// restores original value at the end
struct disableIf
{
    GLOW_NON_COPYABLE(disableIf);

    GLenum state;
    GLboolean prevState;

    disableIf(bool condition, GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        if (condition)
            glDisable(state);
    }
    ~disableIf()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

/// Enables the given state if condition is true (otherwise keeps the same)
/// restores original value at the end
struct enableIf
{
    GLOW_NON_COPYABLE(enableIf);

    GLenum state;
    GLboolean prevState;

    enableIf(bool condition, GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        if (condition)
            glEnable(state);
    }
    ~enableIf()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

/// Disables the given state if condition is true (otherwise enables it)
/// restores original value at the end
struct onlyDisableIf
{
    GLOW_NON_COPYABLE(onlyDisableIf);

    GLenum state;
    GLboolean prevState;

    onlyDisableIf(bool condition, GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        if (condition)
            glDisable(state);
        else
            glEnable(state);
    }
    ~onlyDisableIf()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

/// Enables the given state if condition is true (otherwise disables it)
/// restores original value at the end
struct onlyEnableIf
{
    GLOW_NON_COPYABLE(onlyEnableIf);

    GLenum state;
    GLboolean prevState;

    onlyEnableIf(bool condition, GLenum state) : state(state)
    {
        glGetBooleanv(state, &prevState);
        if (condition)
            glEnable(state);
        else
            glDisable(state);
    }
    ~onlyEnableIf()
    {
        if (prevState)
            glEnable(state);
        else
            glDisable(state);
    }
};

struct debugGroup
{
    GLOW_NON_COPYABLE(debugGroup);

    debugGroup(std::string const& name, GLuint id = 0)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, -1, name.c_str());
    }

    ~debugGroup() { glPopDebugGroup(); }
};

#define GLOW_SCOPED_ENUM_VALUED_GL_FUNC(name, func, getName) \
    struct name                                              \
    {                                                        \
        GLOW_NON_COPYABLE(name);                             \
                                                             \
        GLenum prevVal;                                      \
                                                             \
        name(GLenum val)                                     \
        {                                                    \
            glGetIntegerv(getName, (GLint*)&prevVal);        \
            func(val);                                       \
        }                                                    \
        ~name() { func(prevVal); }                           \
    } // force ;
#define GLOW_SCOPED_BOOL_VALUED_GL_FUNC(name, func, getName) \
    struct name                                              \
    {                                                        \
        GLOW_NON_COPYABLE(name);                             \
                                                             \
        GLboolean prevVal;                                   \
                                                             \
        name(bool val)                                       \
        {                                                    \
            glGetBooleanv(getName, &prevVal);                \
            func((GLboolean)val);                            \
        }                                                    \
        ~name() { func(prevVal); }                           \
    } // force ;
#define GLOW_SCOPED_FLOAT_VALUED_GL_FUNC(name, func, getName) \
    struct name                                               \
    {                                                         \
        GLOW_NON_COPYABLE(name);                              \
                                                              \
        GLfloat prevVal;                                      \
                                                              \
        name(float val)                                       \
        {                                                     \
            glGetFloatv(getName, &prevVal);                   \
            func(val);                                        \
        }                                                     \
        ~name() { func(prevVal); }                            \
    } // force ;
#define GLOW_SCOPED_DOUBLE_VALUED_GL_FUNC(name, func, getName) \
    struct name                                                \
    {                                                          \
        GLOW_NON_COPYABLE(name);                               \
                                                               \
        GLdouble prevVal;                                      \
                                                               \
        name(double val)                                       \
        {                                                      \
            glGetDoublev(getName, &prevVal);                   \
            func(val);                                         \
        }                                                      \
        ~name() { func(prevVal); }                             \
    } // force ;

/// Calls the associated gl function with given value
/// restores original value at the end
GLOW_SCOPED_ENUM_VALUED_GL_FUNC(cullFace, glCullFace, GL_CULL_FACE_MODE);
GLOW_SCOPED_ENUM_VALUED_GL_FUNC(frontFace, glFrontFace, GL_FRONT_FACE);
GLOW_SCOPED_ENUM_VALUED_GL_FUNC(logicOp, glLogicOp, GL_LOGIC_OP_MODE);
GLOW_SCOPED_ENUM_VALUED_GL_FUNC(blendEquation, glBlendEquation, GL_BLEND_EQUATION_RGB);
GLOW_SCOPED_ENUM_VALUED_GL_FUNC(depthFunc, glDepthFunc, GL_DEPTH_FUNC);

GLOW_SCOPED_BOOL_VALUED_GL_FUNC(depthMask, glDepthMask, GL_DEPTH_WRITEMASK);

GLOW_SCOPED_DOUBLE_VALUED_GL_FUNC(clearDepth, glClearDepth, GL_DEPTH_CLEAR_VALUE);
GLOW_SCOPED_FLOAT_VALUED_GL_FUNC(lineWidth, glLineWidth, GL_LINE_WIDTH);
GLOW_SCOPED_FLOAT_VALUED_GL_FUNC(pointSize, glPointSize, GL_POINT_SIZE);

struct blendFunc
{
    GLOW_NON_COPYABLE(blendFunc);

    GLenum prevSrcRGB;
    GLenum prevDstRGB;
    GLenum prevSrcAlpha;
    GLenum prevDstAlpha;

    blendFunc(GLenum srcFunc, GLenum dstFunc)
    {
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&prevSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&prevDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&prevSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&prevDstAlpha);
        glBlendFunc(srcFunc, dstFunc);
    }

    blendFunc(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
    {
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&prevSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&prevDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&prevSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&prevDstAlpha);
        glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }
    ~blendFunc() { glBlendFuncSeparate(prevSrcRGB, prevDstRGB, prevSrcAlpha, prevDstAlpha); }
};

struct clearColor
{
    GLOW_NON_COPYABLE(clearColor);

    GLfloat color[4];

    clearColor(float r, float g, float b, float a)
    {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
        glClearColor(r, g, b, a);
    }
    ~clearColor() { glClearColor(color[0], color[1], color[2], color[3]); }
};

struct blendColor
{
    GLOW_NON_COPYABLE(blendColor);

    GLfloat color[4];

    blendColor(float r, float g, float b, float a)
    {
        glGetFloatv(GL_BLEND_COLOR, color);
        glBlendColor(r, g, b, a);
    }
    ~blendColor() { glBlendColor(color[0], color[1], color[2], color[3]); }
};

struct colorMask
{
    GLOW_NON_COPYABLE(colorMask);

    GLboolean color[4];

    colorMask(bool r, bool g, bool b, bool a)
    {
        glGetBooleanv(GL_COLOR_WRITEMASK, color);
        glColorMask(r, g, b, a);
    }
    ~colorMask() { glColorMask(color[0], color[1], color[2], color[3]); }
};

struct viewport
{
    GLOW_NON_COPYABLE(viewport);

    GLint prevView[4];

    viewport(int x, int y, int w, int h)
    {
        glGetIntegerv(GL_VIEWPORT, prevView);
        glViewport(x, y, w, h);
    }
    ~viewport() { glViewport(prevView[0], prevView[1], prevView[2], prevView[3]); }
};

struct polygonMode
{
    GLOW_NON_COPYABLE(polygonMode);

    GLint prevMode[2];

    polygonMode(GLenum face, GLenum mode)
    {
        glGetIntegerv(GL_POLYGON_MODE, prevMode);
        glPolygonMode(face, mode);
    }
    ~polygonMode()
    {
        glPolygonMode(GL_FRONT, prevMode[0]);
        glPolygonMode(GL_BACK, prevMode[1]);
    }
};

struct sampleCoverage
{
    GLOW_NON_COPYABLE(sampleCoverage);

    GLfloat prevVal;
    GLboolean prevInvert;

    sampleCoverage(float value, bool invert)
    {
        glGetFloatv(GL_SAMPLE_COVERAGE_VALUE, &prevVal);
        glGetBooleanv(GL_SAMPLE_COVERAGE_INVERT, &prevInvert);
        glSampleCoverage(value, invert);
    }
    ~sampleCoverage() { glSampleCoverage(prevVal, prevInvert); }
};

struct polygonOffset
{
    GLOW_NON_COPYABLE(polygonOffset);

    GLfloat prevFactor;
    GLfloat prevUnits;

    polygonOffset(float factor, float units)
    {
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &prevFactor);
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &prevUnits);
        glPolygonOffset(factor, units);
    }
    ~polygonOffset() { glPolygonOffset(prevFactor, prevUnits); }
};

struct pixelStore
{
    GLOW_NON_COPYABLE(pixelStore);

    GLenum mode;
    GLint prevSetting;

    pixelStore(GLenum mode, GLint setting) : mode(mode)
    {
        glGetIntegerv(mode, &prevSetting);
        glPixelStorei(mode, setting);
    }
    ~pixelStore() { glPixelStorei(mode, prevSetting); }
};
struct packAlignment : pixelStore
{
    packAlignment(int value) : pixelStore(GL_PACK_ALIGNMENT, value) {}
};
struct packSwapBytes : pixelStore
{
    packSwapBytes(bool value) : pixelStore(GL_PACK_SWAP_BYTES, value) {}
};
struct packLsbFirst : pixelStore
{
    packLsbFirst(bool value) : pixelStore(GL_PACK_LSB_FIRST, value) {}
};
struct packRowLength : pixelStore
{
    packRowLength(int value) : pixelStore(GL_PACK_ROW_LENGTH, value) {}
};
struct packImageHeight : pixelStore
{
    packImageHeight(int value) : pixelStore(GL_PACK_IMAGE_HEIGHT, value) {}
};
struct packSkipRows : pixelStore
{
    packSkipRows(int value) : pixelStore(GL_PACK_SKIP_ROWS, value) {}
};
struct packSkipPixels : pixelStore
{
    packSkipPixels(int value) : pixelStore(GL_PACK_SKIP_PIXELS, value) {}
};
struct packSkipImages : pixelStore
{
    packSkipImages(int value) : pixelStore(GL_PACK_SKIP_IMAGES, value) {}
};
struct unpackAlignment : pixelStore
{
    unpackAlignment(int value) : pixelStore(GL_UNPACK_ALIGNMENT, value) {}
};
struct unpackSwapBytes : pixelStore
{
    unpackSwapBytes(bool value) : pixelStore(GL_UNPACK_SWAP_BYTES, value) {}
};
struct unpackLsbFirst : pixelStore
{
    unpackLsbFirst(bool value) : pixelStore(GL_UNPACK_LSB_FIRST, value) {}
};
struct unpackRowLength : pixelStore
{
    unpackRowLength(int value) : pixelStore(GL_UNPACK_ROW_LENGTH, value) {}
};
struct unpackImageHeight : pixelStore
{
    unpackImageHeight(int value) : pixelStore(GL_UNPACK_IMAGE_HEIGHT, value) {}
};
struct unpackSkipRows : pixelStore
{
    unpackSkipRows(int value) : pixelStore(GL_UNPACK_SKIP_ROWS, value) {}
};
struct unpackSkipPixels : pixelStore
{
    unpackSkipPixels(int value) : pixelStore(GL_UNPACK_SKIP_PIXELS, value) {}
};
struct unpackSkipImages : pixelStore
{
    unpackSkipImages(int value) : pixelStore(GL_UNPACK_SKIP_IMAGES, value) {}
};
}
}
