#pragma once

//! Includes System
#include <cstdint>


namespace aion
{
namespace NetMessageOptions
{
typedef uint8_t Type;
const Type compressed = 1 << 1;
const Type RESERVED = 1 << 7;
const Type UNINITIALIZED = 0xFF;
}
}
