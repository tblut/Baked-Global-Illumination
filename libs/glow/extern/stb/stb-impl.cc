// stb image produces these warnings and it is unlikely to be fixed
#pragma GCC diagnostic ignored "-Wmisleading-indentation"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void dummy_stb_func() { }
