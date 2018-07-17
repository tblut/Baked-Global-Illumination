#pragma once

#include "macro_join.hh"

/** GETTER(name) SETTER(name) PROPERTY(name)
 * as non-typed getter/setter macros
 *
 * Usage:
 * class Foo {
 * private:
 *    int mBar;
 *    bool mFinished;
 *
 * public:
 *    PROPERTY(Bar); // generates getBar() and setBar(...) with const& set and get
 *    PROPERTY_IS(Finished); // isFinished() and setFinished()
 * };
 *
 * CAUTION: these macros can only be used _after_ the member is declared (due to type deduction)
 */

#define AION_GETTER(name)                                                      \
    auto get##name() const->decltype(m##name) const & { return m##name; } \
    friend class AION_MACRO_JOIN(___get_, __COUNTER__)

#define AION_SETTER(name)                                                    \
    void set##name(decltype(m##name) const& value) { m##name = value; } \
    friend class AION_MACRO_JOIN(___set_, __COUNTER__)

#define AION_GETTER_IS(name)                                                  \
    auto is##name() const->decltype(m##name) const & { return m##name; } \
    friend class AION_MACRO_JOIN(___get_is_, __COUNTER__)

#define AION_PROPERTY(name) \
    AION_GETTER(name);      \
    AION_SETTER(name)

#define AION_PROPERTY_IS(name) \
    AION_GETTER_IS(name);      \
    AION_SETTER(name)
