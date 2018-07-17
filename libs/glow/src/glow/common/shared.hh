#pragma once

#include <memory>

/** Shared definitions
 *
 * Can also be used for forward declarations
 *
 * Usage:
 *
 * GLOW_SHARED(class, X);
 * GLOW_SHARED(struct, Y);
 *
 * // now SharedX is usable
 * // and SharedY... for struct Y
 */

#define GLOW_SHARED(type, class_or_struct_name) \
    type class_or_struct_name;                  \
                                                \
    using Shared##class_or_struct_name = std::shared_ptr<class_or_struct_name> // force ;
