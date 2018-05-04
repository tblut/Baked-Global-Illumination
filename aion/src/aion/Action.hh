#pragma once

#include "common/macro_join.hh"

// pretty function
#ifdef _MSC_VER
#define AION_PRETTY_FUNC __FUNCTION__
#else
#define AION_PRETTY_FUNC __PRETTY_FUNCTION__
#endif

#include "ActionLabel.hh"

/**
  ACTION-based high-performance profiling

    * Designed for minimal overhead recording
    * Analysis will probably convert the storage
    * Records one tree per thread (not mixed, no locking)
    * Only c-string constants allowed for name
    * ACTIONs always run until the scope ends
      (multiple ACTIONs in the same scope are allowed but not really recommended)
    * Supports recursion
    * Supports run-time evaluation (of the current thread)
    * Requires 24 bytes per executed ACTION
    * No run-time clearing yet

  Usage:
    void foo() {
        ACTION(); // anon action

        // do stuff

        {
            ACTION("with label"); // named action

            // nested stuff
        }

        // do more stuff
    }

  Implementation notes:
    * Per Action storage requirements
        * starttime: uint64_t
        * label idx: uint32_t
        * endtime: uint64_t
        * end label: uint32_t
        * => 8 + 4 + 8 + 4 = 24 bytes
 */

#define ACTION(...)                                                                                                         \
    (void) __VA_ARGS__ " has to be a string literal";                                                                       \
    static aion::ActionLabel AION_MACRO_JOIN(__action_label_, __LINE__){__FILE__, __LINE__, AION_PRETTY_FUNC, "" __VA_ARGS__}; \
    aion::ActionScopeGuard AION_MACRO_JOIN(__action_, __LINE__) { &AION_MACRO_JOIN(__action_label_, __LINE__) }
namespace aion
{
class ActionScopeGuard
{
private:
    ActionLabel *label;

public:
    ActionScopeGuard(ActionLabel *l) : label(l) { l->startEntry(); }
    ~ActionScopeGuard() { label->endEntry(); }
};
}
