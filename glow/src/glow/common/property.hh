#pragma once

#include "macro_join.hh"

/** GLOW_GETTER(name) GLOW_SETTER(name) GLOW_PROPERTY(name)
 * as non-typed getter/setter macros
 *
 * Usage:
 * class Foo {
 * private:
 *    int mBar;
 *    bool mFinished;
 *
 * public:
 *    GLOW_PROPERTY(Bar); // generates getBar() and setBar(...) with const& set and get
 *    GLOW_PROPERTY_IS(Finished); // isFinished() and setFinished()
 * };
 *
 * CAUTION: these macros can only be used _after_ the member is declared (due to type deduction)
 */

#define GLOW_GETTER(name)                                                 \
    auto get##name() const->decltype(m##name) const & { return m##name; } \
    friend class GLOW_MACRO_JOIN(___get_, __COUNTER__)

#define GLOW_SETTER(name)                                               \
    void set##name(decltype(m##name) const& value) { m##name = value; } \
    friend class GLOW_MACRO_JOIN(___set_, __COUNTER__)

#define GLOW_GETTER_IS(name)                                             \
    auto is##name() const->decltype(m##name) const & { return m##name; } \
    friend class GLOW_MACRO_JOIN(___get_is_, __COUNTER__)

#define GLOW_PROPERTY(name) \
    GLOW_GETTER(name);      \
    GLOW_SETTER(name)

#define GLOW_PROPERTY_IS(name) \
    GLOW_GETTER_IS(name);      \
    GLOW_SETTER(name)
