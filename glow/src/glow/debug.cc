#include "debug.hh"

#include "common/log.hh"
#include "fmt/format.hh"

void APIENTRY glow::DebugMessageCallback(GLenum source,
                                         GLenum type,
                                         GLuint id,
                                         GLenum severity,
                                         GLsizei length,
                                         const GLchar *message,
#ifdef GLOW_ACGL_COMPAT
                                         void *userParam
#else
                                         const void *userParam
#endif
                                         )
{
    // filter too verbose messages
    switch (id)
    {
    // NVidia
    case 131185: // Buffer detailed info 'will use VIDEO memory as the source for buffer object'
    case 131218: // Program/shader state performance warning: Fragment Shader is going to be recompiled because the
                 // shader key based on GL state mismatches.
    case 131186: // Buffer performance warning: Buffer object (bound to GL_SHADER_STORAGE_BUFFER, and
                 // GL_SHADER_STORAGE_BUFFER (0), usage hint is GL_STATIC_DRAW) is being copied/moved from VIDEO memory
                 // to HOST memory.
        return;
    }

    // ignore push/pop
    if (source == GL_DEBUG_SOURCE_APPLICATION && type == GL_DEBUG_TYPE_PUSH_GROUP)
        return;
    if (source == GL_DEBUG_SOURCE_APPLICATION && type == GL_DEBUG_TYPE_POP_GROUP)
        return;

    const char *sSource;
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sSource = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sSource = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sSource = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sSource = "3rd Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sSource = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sSource = "Other";
        break;
    default:
        sSource = "Unknown";
        break;
    }

    const char *sSeverity;
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        sSeverity = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        sSeverity = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        sSeverity = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        sSeverity = "Notification";
        break;
    default:
        sSeverity = "Unknown";
        break;
    }

    const char *sType;
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        sType = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        sType = "Deprecated";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        sType = "Undefined";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        sType = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        sType = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        sType = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        sType = "Push";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        sType = "Pop";
        break;
    case GL_DEBUG_TYPE_OTHER:
        sType = "Other";
        break;
    default:
        sType = "Unknown";
        break;
    }

    error() << fmt::format("[OpenGL][{}][{}][{}][{}] {}", sType, sSeverity, sSource, id, message);
}
