#pragma once

#include <glow/common/macro_join.hh>
#include <glow/common/pretty_function.hh>

#include "GlowActionLabel.hh"

#define GLOW_ACTION(...)                                                                                               \
    (void) __VA_ARGS__ " has to be a string literal";                                                                  \
    static glow::GlowActionLabel GLOW_MACRO_JOIN(__glow_action_label_, __LINE__){__FILE__, __LINE__,                   \
                                                                                 __PRETTY_FUNCTION__, "" __VA_ARGS__}; \
    glow::GlowActionScopeGuard GLOW_MACRO_JOIN(__glow_action_, __LINE__)                                               \
    {                                                                                                                  \
        &GLOW_MACRO_JOIN(__glow_action_label_, __LINE__)                                                               \
    }

namespace glow
{
class GlowActionScopeGuard
{
private:
    GlowActionLabel *label;

public:
    GlowActionScopeGuard(GlowActionLabel *l) : label(l) { l->startEntry(); }
    ~GlowActionScopeGuard() { label->endEntry(); }
};
}
