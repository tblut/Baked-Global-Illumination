#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glow/common/property.hh>
#include <glow/common/shared.hh>
#include <glow/fwd.hh>

#include <glow-extras/timing/PerformanceTimer.hh>

struct GLFWwindow;
struct CTwBar;
typedef struct CTwBar TwBar; // structure CTwBar is not exposed.

namespace glow
{
namespace pipeline
{
struct RenderPass;
GLOW_SHARED(class, RenderingPipeline);
}
namespace camera
{
GLOW_SHARED(class, GenericCamera);
}
namespace debugging
{
GLOW_SHARED(class, DebugRenderer);
}

namespace glfw
{
enum class CursorMode
{
    /// normal behavior
    Normal,
    /// normal behavior but hardware cursor is hidden
    Hidden,
    /// virtual unrestricted cursor, real cursor is hidden and locked to center
    Disabled,
};

/**
 * @brief The GlfwApp can be used to efficiently build small sample applications based on glfw
 *
 * Derive your own class from GlfwApp and override the functions you need:
 *   - init(...): initialize and allocate all your resources and objects
 *   - update(...): called with a constant rate (default 60 Hz, configurable) before rendering
 *   - render(...): called as fast as possible (affected by vsync)
 *   - renderPass(...): if rendering pipeline enabled (default), default render(...) will call this (RECOMMENDED)
 *        NOTE: if you use debug()->renderXYZ, do so BEFORE call to base
 *   - onResize(...): called when window is resized
 *   - onClose(...): called when app is closed
 *   - input: see onKey/onChar/onMouseXYZ/onFileDrop (return true if you handled the event, if base returned true, you
 *                                                    should probably return as well)
 * be sure to call base function unless you know what you do!
 *
 * Additional important functions:
 *   - setUpdateRate(...): set the update rate
 *   - window(): get the GLFW window
 *   - tweakbar(): get the AntTweakBar instance
 *   - setWindowWidth/Height(...): set initial window size before run(...)
 *
 * Notes:
 *   - if you use primitive/occlusion queries, use setQueryStats(false);
 *   - overwrite onResetView if you want a different default view
 *
 * Defaults:
 *   - A GenericCamera with input handling on LMB/RMB/WASD/... (setUseDefaultXYZ to configure)
 *
 * Usage:
 * int main(int argc, char *argv[])
 * {
 *   MyGlfwApp app;
 *   return app.run(argc, argv); // automatically sets up GLOW and GLFW and everything
 * }
 */
class GlfwApp
{
private:
    std::string mTitle = "GLFW/GLOW Application"; ///< window title

    double mUpdateRate = 60; ///< rate at which update(...) is called
    int mMaxFrameSkip = 4;   ///< maximum number of update(...) steps that are performed without rendering

    GLFWwindow* mWindow = nullptr; ///< current GLFW window
    TwBar* mTweakbar = nullptr;    ///< main tweakbar window

    int mWindowWidth = 1280; ///< window width, only set before run!
    int mWindowHeight = 720; ///< window height, only set before run!

    bool mDumpTimingsOnShutdown = true; ///< if true, dumps AION timings on shutdown

    double mMouseX = -1.0; ///< cursor X in pixels
    double mMouseY = -1.0; ///< cursor Y in pixels

    CursorMode mCursorMode = CursorMode::Normal; ///< cursor mode

    bool mDrawTweakbars = true; ///< if true, draws tweakbars

    bool mVSync = true; ///< if true, enables vsync

    double mOutputStatsInterval = 5.0; ///< number of seconds between stats output (0.0 for never)
    bool mQueryStats = true;           ///< if true, queries stats (vertices, fragments, ...)

    bool mUseDefaultCamera = true;              ///< if true, uses default camera
    bool mUseDefaultCameraHandling = true;      ///< if true, implements default cam handling
    bool mUseDefaultCameraHandlingLeft = true;  ///< if true, activates left mouse button handling
    bool mUseDefaultCameraHandlingRight = true; ///< if true, activates right mouse button handling

    bool mUseRenderingPipeline = false; ///< if true, uses default rendering pipeline setup (requires default cam)

    double mCurrentTime = 0.0; ///< current frame time (starts with 0)

    double mDoubleClickTime = 0.35f; ///< max number of seconds for multi clicks
    int mClickCount = 0;             ///< current click count
    int mClickButton = -1;           ///< last clicked button
    glm::vec2 mClickPos;             ///< last clicked position
    timing::SystemTimer mClickTimer; ///< click timing

    // Default graphics
private:
    camera::SharedGenericCamera mCamera;         ///< default camera
    pipeline::SharedRenderingPipeline mPipeline; ///< default pipeline

    SharedPrimitiveQuery mPrimitiveQuery; ///< nr of primitives per frame
    SharedOcclusionQuery mOcclusionQuery; ///< nr of pixels per frame
    SharedTimerQuery mRenderStartQuery;   ///< start timestamp
    SharedTimerQuery mRenderEndQuery;     ///< end timestamp

    debugging::SharedDebugRenderer mDebugRenderer; ///< debug rendering

    // Camera handling
private:
    bool mMouseLeft = false;
    bool mMouseRight = false;

    float mCameraMoveSpeed = 15.0f;      ///< in units/s
    float mCameraMoveSpeedFactor = 5.0f; ///< factor of move speed if shift is pressed
    float mCameraTurnSpeed = 5.0f;       ///< in radians/total screen
    float mCameraScrollSpeed = 5.0f;     ///< in % per scroll

    double mMouseLastX = -1.0;
    double mMouseLastY = -1.0;

public:
    GLOW_PROPERTY(UpdateRate);
    GLOW_PROPERTY(MaxFrameSkip);
    GLOW_GETTER(Title);
    GLOW_PROPERTY(WindowWidth);
    GLOW_PROPERTY(WindowHeight);
    GLOW_PROPERTY(DumpTimingsOnShutdown);
    GLOW_PROPERTY(CursorMode);
    GLOW_PROPERTY(DrawTweakbars);
    GLOW_PROPERTY(VSync);
    GLOW_PROPERTY(OutputStatsInterval);
    GLOW_PROPERTY(QueryStats);
    GLOW_PROPERTY(UseRenderingPipeline);
    GLOW_PROPERTY(DoubleClickTime);
    float getCurrentTime() const { return mCurrentTime; }
    double getCurrentTimeD() const { return mCurrentTime; }
    GLOW_GETTER(Camera);
    GLOW_GETTER(Pipeline);

    GLOW_PROPERTY(UseDefaultCamera);
    GLOW_PROPERTY(UseDefaultCameraHandling);
    GLOW_PROPERTY(UseDefaultCameraHandlingLeft);
    GLOW_PROPERTY(UseDefaultCameraHandlingRight);
    GLOW_PROPERTY(CameraMoveSpeed);
    GLOW_PROPERTY(CameraTurnSpeed);
    GLOW_PROPERTY(CameraScrollSpeed);

    void setTitle(std::string const& title);

    /// Only works with setUseRenderingPipeline(true)
    debugging::SharedDebugRenderer const& debug() const { return mDebugRenderer; }

    glm::vec2 getMousePosition() const { return {mMouseX, mMouseY}; }
    GLFWwindow* window() const { return mWindow; }
    TwBar* tweakbar() const { return mTweakbar; }

public:
    /// sets the current clipboard content
    void setClipboardString(std::string const& s) const;
    /// gets the current clipboard content
    std::string getClipboardString() const;

    /// Returns true iff the specified mouse button (GLFW_...) is pressed
    bool isMouseButtonPressed(int button) const;
    /// Returns true iff the specified key (GLFW_...) is pressed
    bool isKeyPressed(int key) const;

    /// Returns true iff the app should be closed
    /// Defaults to glfw window closed
    bool shouldClose() const;

protected:
    /// Called once GLOW is initialized. Allocated your resources and init your logic here.
    virtual void init();
    /// Called with at 1 / getUpdateRate() Hz (timestep)
    virtual void update(float elapsedSeconds);
    /// Called as fast as possible for rendering (elapsedSeconds is not fixed here)
    virtual void render(float elapsedSeconds);
    /// When using the builtin rendering pipeline, this is called for every pass in every render step
    virtual void renderPass(pipeline::RenderPass const& pass, float elapsedSeconds);
    /// Called once in the beginning after (init) and whenever the window size changed
    virtual void onResize(int w, int h);
    /// Called at the end, when application is closed
    virtual void onClose();

    /// Called whenever a key is pressed
    virtual bool onKey(int key, int scancode, int action, int mods);
    /// Called whenever a character is entered (unicode)
    virtual bool onChar(unsigned int codepoint, int mods);
    /// Called whenever the mouse position changes
    virtual bool onMousePosition(double x, double y);
    /// Called whenever a mouse button is pressed (clickCount is 1 for single clicks, 2 for double, 3+ for multi)
    virtual bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount);
    /// Called whenever the mouse is scrolled
    virtual bool onMouseScroll(double sx, double sy);
    /// Called whenever the mouse enters the window
    virtual bool onMouseEnter();
    /// Called whenever the mouse leaves the window
    virtual bool onMouseExit();
    /// Called whenever the window gets focus
    virtual bool onFocusGain();
    /// Called whenever the window loses focus
    virtual bool onFocusLost();
    /// Called whenever files are dropped (drag'n'drop), parameters is file paths
    virtual bool onFileDrop(std::vector<std::string> const& files);

    /// Called when view should be reset
    virtual void onResetView();

    /// Blocking call that executes the complete main loop
    virtual void mainLoop();

private:
    void internalOnMouseButton(double x, double y, int button, int action, int mods);

protected:
    /// performs glfw polling
    void updateInput();

    /// should be called before rendering
    void beginRender();
    /// should be called after rendering
    /// calls swapBuffers
    void endRender();

    /// Blocks the thread for a given number of milliseconds
    void sleepSeconds(double seconds) const;

public:
    /// Initializes GLFW and GLOW, and runs until window is closed
    int run(int argc, char* argv[]);

public:
    virtual ~GlfwApp(); // virtual dtor
};
}
}
