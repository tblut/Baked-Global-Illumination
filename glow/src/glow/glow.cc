#include "glow.hh"

#include <cassert>

#include "gl.hh"
#include "debug.hh"
#include "limits.hh"

#include "common/log.hh"
#include "common/thread_local.hh"

#ifdef GLOW_ACGL_COMPAT
#include <ACGL/OpenGL/Tools.hh>
#endif

namespace
{
GLOW_THREADLOCAL bool _isGlowInitialized = false;
}

using namespace glow;

glow::_glowGLVersion glow::OGLVersion;

bool glow::initGLOW()
{
    if (_isGlowInitialized)
        return true;

#ifdef GLOW_ACGL_COMPAT
    // assume already initialized
    OGLVersion.major = ACGL::OpenGL::getOpenGLMajorVersionNumber();
    OGLVersion.minor = ACGL::OpenGL::getOpenGLMinorVersionNumber();
#else
    if (!gladLoadGL())
        return false;

    OGLVersion.major = GLVersion.major;
    OGLVersion.minor = GLVersion.minor;
#endif
    OGLVersion.total = OGLVersion.major * 10 + OGLVersion.minor;
    info() << "Loaded OpenGL Version " << OGLVersion.major << "." << OGLVersion.minor;

    // install debug message
    if (glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(DebugMessageCallback, nullptr);
    }

    // BEFORE limits
    _isGlowInitialized = true;

    // update limits
    limits::update();

    // restore defaults
    restoreDefaultOpenGLState();

    return true;
}

#ifdef GLOW_PERFORM_VALIDATION
void glow::checkValidGLOW()
{
    assert(_isGlowInitialized && "GLOW is not initialized OR called from wrong thread.");
    if (!_isGlowInitialized) // for release
         glow::error() << "ERROR: GLOW is not initialized OR called from wrong thread.";
}
#endif
