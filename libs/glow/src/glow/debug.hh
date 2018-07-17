#pragma once

#include "gl.hh"

namespace glow
{
/// Each time a debug message is generated the debug callback function will be invoked with source, type, id, and
/// severity associated with the message, and length set to the length of debug message whose character string is in the
/// array pointed to by message userParam will be set to the value passed in the userParam parameter to the most recent
/// call to glDebugMessageCallback.
void APIENTRY DebugMessageCallback(GLenum source,
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
                                   );
}
