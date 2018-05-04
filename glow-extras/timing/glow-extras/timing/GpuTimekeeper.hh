#pragma once

#include <glow/gl.hh>
#include <glow/fwd.hh>

#include <glow/common/shared.hh>
#include <glow/common/macro_join.hh>

#include <vector>
#include <string>

namespace glow
{
namespace timing
{
#define GLOW_TIMEKEEPER_FRAME(TIMEKEEPER) \
    glow::timing::GpuTimekeeper::RaiiFrame GLOW_MACRO_JOIN(_glow_timing_frame_, __LINE__)(TIMEKEEPER)
#define GLOW_TIMEKEEPER_ACTION(TIMEKEEPER, NAME) \
    glow::timing::GpuTimekeeper::RaiiAction GLOW_MACRO_JOIN(_glow_timing_action_, __LINE__)(TIMEKEEPER, NAME)

/**
 * @brief The GpuTimekeeper is used to analyze gpu frame timings
 */
class GpuTimekeeper
{
private:
    GLOW_SHARED(struct, Action);
    GLOW_SHARED(struct, Frame);
    struct Action
    {
        std::string name;
        int depth = 0;

        GLint queryStartID = -1;
        GLint queryEndID = -1;

        GLuint64 startTime;
        GLuint64 endTime;

        std::vector<SharedAction> children;

        Frame* frame = nullptr;

        double durationMS() const { return (endTime - startTime) / 1000000.; }

        void print() const;
    };

    struct Frame
    {
        SharedAction root;

        int openQueries = 0;
    };

public:
    struct RaiiFrame
    {
        GpuTimekeeper& keeper;

        RaiiFrame(GpuTimekeeper& keeper) : keeper(keeper) { keeper.startFrame(); }
        ~RaiiFrame() { keeper.endFrame(); }
    };
    struct RaiiAction
    {
        GpuTimekeeper& keeper;

        RaiiAction(GpuTimekeeper& keeper, std::string const& name) : keeper(keeper) { keeper.startAction(name); }
        ~RaiiAction() { keeper.endAction(); }
    };

private:
    std::vector<SharedFrame> mFrames;

    SharedFrame mCurrentFrame;
    SharedFrame mCompleteFrame;
    std::vector<SharedAction> mActionStack;

    std::vector<SharedAction> mUnfinishedActions;
    std::vector<GLuint> mUnusedQueries;

public:
    GpuTimekeeper();

    /// Starts frame timing
    void startFrame();
    /// Starts frame timing
    void endFrame();

    /// Starts a sub-action
    void startAction(std::string const& name);
    /// Ends a sub-action
    void endAction();

    void update();

    void print() const;

private:
    GLuint getQuery();
    void returnQuery(GLuint query);
};
}
}
