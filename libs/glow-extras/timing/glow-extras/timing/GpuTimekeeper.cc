#include "GpuTimekeeper.hh"

#include <glow/common/log.hh>

#include <cassert>

using namespace glow;
using namespace glow::timing;

GpuTimekeeper::GpuTimekeeper()
{
}

void GpuTimekeeper::startFrame()
{
    assert(mCurrentFrame == nullptr && "cannot start a new frame while current one is still running");

    mCurrentFrame = std::make_shared<Frame>();

    startAction("frame");
}

void GpuTimekeeper::endFrame()
{
    endAction();

    assert(mCurrentFrame && "cannot end non-started frame");

    mFrames.push_back(mCurrentFrame);
    mCurrentFrame = nullptr;
}

void GpuTimekeeper::startAction(const std::string& name)
{
    assert(mCurrentFrame && "cannot start action with no frame");

    auto action = std::make_shared<Action>();
    action->name = name;
    action->frame = mCurrentFrame.get();

    if (mActionStack.empty())
        mCurrentFrame->root = action;
    else
    {
        mActionStack.back()->children.push_back(action);
        action->depth = mActionStack.back()->depth + 1;
    }

    mActionStack.push_back(action);

    action->queryStartID = getQuery();
    action->queryEndID = getQuery();

    glQueryCounter(action->queryStartID, GL_TIMESTAMP);

    mCurrentFrame->openQueries += 2;
}

void GpuTimekeeper::endAction()
{
    assert(mCurrentFrame && "cannot end action with no frame");
    assert(!mActionStack.empty() && "cannot end action on empty stack");

    auto action = mActionStack.back();

    glQueryCounter(action->queryEndID, GL_TIMESTAMP);

    mUnfinishedActions.push_back(action);
    mActionStack.pop_back();
}

void GpuTimekeeper::update()
{
    for (auto i = (int)mUnfinishedActions.size() - 1; i >= 0; --i)
    {
        auto const& a = mUnfinishedActions[i];

        if (a->queryStartID != -1)
        {
            GLuint resultAvailable;
            glGetQueryObjectuiv(a->queryStartID, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);
            if (resultAvailable)
            {
                glGetQueryObjectui64v(a->queryStartID, GL_QUERY_RESULT, &a->startTime);
                returnQuery(a->queryStartID);
                a->queryStartID = -1;

                a->frame->openQueries--;
            }
        }

        if (a->queryEndID != -1)
        {
            GLuint resultAvailable;
            glGetQueryObjectuiv(a->queryEndID, GL_QUERY_RESULT_AVAILABLE, &resultAvailable);
            if (resultAvailable)
            {
                glGetQueryObjectui64v(a->queryEndID, GL_QUERY_RESULT, &a->endTime);
                returnQuery(a->queryEndID);
                a->queryEndID = -1;

                a->frame->openQueries--;
            }
        }

        if (a->queryStartID == -1 && a->queryEndID == -1)
        {
            mUnfinishedActions.erase(mUnfinishedActions.begin() + i);
        }
    }

    mCompleteFrame = nullptr;
    for (auto i = (int)mFrames.size() - 1; i >= 0; --i)
    {
        auto const& f = mFrames[i];

        assert(f->openQueries >= 0 && "inconsistent query count");
        if (f->openQueries == 0)
        {
            if (!mCompleteFrame)
                mCompleteFrame = f;
            else
                mFrames.erase(mFrames.begin() + i);
        }
    }
}

void GpuTimekeeper::print() const
{
    if (mCompleteFrame)
        mCompleteFrame->root->print();
}

void GpuTimekeeper::Action::print() const
{
    glow::info() << std::string(depth * 2, ' ') << "- " << name << ": " << durationMS() << " ms";

    for (auto const& c : children)
        c->print();
}

GLuint GpuTimekeeper::getQuery()
{
    if (!mUnusedQueries.empty())
    {
        auto query = mUnusedQueries.back();
        mUnusedQueries.pop_back();
        return query;
    }

    GLuint query;
    glGenQueries(1, &query);
    return query;
}

void GpuTimekeeper::returnQuery(GLuint query)
{
    mUnusedQueries.push_back(query);
}
