#pragma once

#include <memory>

/** Shared definitions
 *
 * Can also be used for forward declarations
 *
 * Usage:
 *
 * AION_SHARED(class, X);
 * AION_SHARED(struct, Y);
 *
 * // now SharedX is usable
 * // and ConstSharedX for const X*
 * // and SharedY... for struct Y
 */

#define AION_SHARED(type, class_or_struct_name)                                                 \
    type class_or_struct_name;                                                             \
                                                                                           \
    using Shared##class_or_struct_name = std::shared_ptr<class_or_struct_name>;            \
    using ConstShared##class_or_struct_name = std::shared_ptr<const class_or_struct_name>; \
                                                                                           \
    using Weak##class_or_struct_name = std::weak_ptr<class_or_struct_name>;                \
    using ConstWeak##class_or_struct_name = std::weak_ptr<const class_or_struct_name>;     \
                                                                                           \
    using Unique##class_or_struct_name = std::unique_ptr<class_or_struct_name>;            \
    using ConstUnique##class_or_struct_name = std::unique_ptr<const class_or_struct_name>
