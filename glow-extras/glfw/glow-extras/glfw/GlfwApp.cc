#include "GlfwApp.hh"

#include <AntTweakBar.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <cassert>

#include <chrono>
#include <iostream>
#include <thread>

#include <glow/gl.hh>

#include <GLFW/glfw3.h>

#include <fmt/format.hh>

#include <aion/ActionAnalyzer.hh>

#include <glow/common/log.hh>
#include <glow/common/str_utils.hh>
#include <glow/glow.hh>

#include <glow/util/DefaultShaderParser.hh>

#include <glow/objects/OcclusionQuery.hh>
#include <glow/objects/PrimitiveQuery.hh>
#include <glow/objects/TimerQuery.hh>

#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/debugging/DebugRenderer.hh>
#include <glow-extras/pipeline/RenderingPipeline.hh>

using namespace glow;
using namespace glow::glfw;

static std::string thousandSep(size_t val)
{
    auto s = std::to_string(val);
    auto l = s.size();
    while (l > 3)
    {
        s = s.substr(0, l - 3) + "'" + s.substr(l - 3);
        l -= 3;
    }
    return s;
}

void GlfwApp::setTitle(const std::string &title)
{
    mTitle = title;
    if (mWindow)
        glfwSetWindowTitle(mWindow, title.c_str());
}

void GlfwApp::setClipboardString(const std::string &s) const
{
    glfwSetClipboardString(mWindow, s.c_str());
}

std::string GlfwApp::getClipboardString() const
{
    auto s = glfwGetClipboardString(mWindow);
    return s ? s : "";
}

bool GlfwApp::isMouseButtonPressed(int button) const
{
    return glfwGetMouseButton(mWindow, button) == GLFW_PRESS;
}

bool GlfwApp::isKeyPressed(int key) const
{
    return glfwGetKey(mWindow, key) == GLFW_PRESS;
}

bool GlfwApp::shouldClose() const
{
    return glfwWindowShouldClose(mWindow);
}

void GlfwApp::init()
{
    if (!mUseDefaultCamera && mUseRenderingPipeline)
        glow::error() << "Cannot use default rendering without default camera in GlfwApp.";

    if (mUseDefaultCamera || mUseRenderingPipeline)
    {
        // create camera with some sensible defaults
        mCamera = std::make_shared<camera::GenericCamera>();
        mCamera->setPosition({2, 2, 2});
        mCamera->setTarget({0, 0, 0});
    }

    if (mUseRenderingPipeline)
    {
        // include paths (BEFORE pipeline init)
        DefaultShaderParser::addIncludePath(util::pathOf(__FILE__) + "/../../../pipeline/shader");
        DefaultShaderParser::addIncludePath(util::pathOf(__FILE__) + "/../../../material/shader");
        DefaultShaderParser::addIncludePath(util::pathOf(__FILE__) + "/../../../debugging/shader");

        // set up rendering pipeline
        mPipeline = pipeline::RenderingPipeline::create(mCamera);

        mDebugRenderer = std::make_shared<debugging::DebugRenderer>();
    }

    mPrimitiveQuery = std::make_shared<PrimitiveQuery>();
    mOcclusionQuery = std::make_shared<OcclusionQuery>();
    mRenderStartQuery = std::make_shared<TimerQuery>();
    mRenderEndQuery = std::make_shared<TimerQuery>();
}

void GlfwApp::update(float elapsedSeconds)
{
    if (mCamera && mUseDefaultCameraHandling)
    {
        auto speed = mCameraMoveSpeed;
        if (glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            speed *= mCameraMoveSpeedFactor;

        if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
            mCamera->moveForward(elapsedSeconds * speed);
        if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
            mCamera->moveBack(elapsedSeconds * speed);
        if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
            mCamera->moveLeft(elapsedSeconds * speed);
        if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
            mCamera->moveRight(elapsedSeconds * speed);
    }
}

void GlfwApp::render(float elapsedSeconds)
{
    if (mUseRenderingPipeline)
    {
        assert(mPipeline && "did you forgot to call GlfwApp::init() in your init?");
        mPipeline->render([this, elapsedSeconds](const pipeline::RenderPass &pass) { renderPass(pass, elapsedSeconds); });
    }
}

void GlfwApp::renderPass(const pipeline::RenderPass &pass, float elapsedSeconds)
{
    if (mUseRenderingPipeline)
        debug()->renderPass(pass);
}

void GlfwApp::onResize(int w, int h)
{
    if (mCamera)
        mCamera->resize(w, h);

    if (mPipeline)
        mPipeline->resize(w, h);
}

void GlfwApp::onClose() {}

bool GlfwApp::onKey(int key, int scancode, int action, int mods)
{
    if (TwEventKeyGLFW(mWindow, key, scancode, action, mods))
        return true;

    if (key == GLFW_KEY_HOME && action != GLFW_RELEASE)
    {
        onResetView();
        return true;
    }

    return false;
}

bool GlfwApp::onChar(unsigned int codepoint, int mods)
{
    if (TwEventCharGLFW(mWindow, codepoint))
        return true;

    return false;
}

bool GlfwApp::onMousePosition(double x, double y)
{
    if (TwEventMousePosGLFW(mWindow, x, y))
        return true;

    if (mMouseLastX >= 0.0 && mCamera && mUseDefaultCameraHandling)
    {
        auto shift = glfwGetKey(mWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        auto alt = glfwGetKey(mWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
        auto ctrl = glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;

        auto dx = x - mMouseLastX;
        auto dy = y - mMouseLastY;

        float ax = dx / mWindowWidth * mCameraTurnSpeed;
        float ay = dy / mWindowHeight * mCameraTurnSpeed;

        if (mUseDefaultCameraHandlingRight && mMouseRight && !alt && !ctrl) // from cam
        {
            mCamera->FPSstyleLookAround(ax, ay);
        }
        if (mUseDefaultCameraHandlingLeft && mMouseLeft && !shift && !alt && !ctrl) // around target
        {
            auto fwd = mCamera->getForwardDirection();

            auto azimuth = glm::atan(fwd.z, fwd.x) + ax;
            auto altitude = glm::atan(fwd.y, glm::sqrt(fwd.x * fwd.x + fwd.z * fwd.z)) - ay;
            altitude = glm::clamp(altitude, -0.499f * glm::pi<float>(), 0.499f * glm::pi<float>());

            auto caz = glm::cos(azimuth);
            auto saz = glm::sin(azimuth);
            auto cal = glm::cos(altitude);
            auto sal = glm::sin(altitude);

            auto newFwd = glm::vec3(cal * caz,  // x
                                    sal,        // y
                                    cal * saz); // z

            auto target = mCamera->getTarget();

            auto dis = mCamera->getLookAtDistance();
            mCamera->setPosition(target - newFwd * dis);
            mCamera->setTarget(target, {0, 1, 0});
        }
    }

    mMouseLastX = x;
    mMouseLastY = y;

    return false;
}

bool GlfwApp::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        mMouseLeft = false;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        mMouseRight = false;

    if (TwEventMouseButtonGLFW(mWindow, button, action, mods))
        return true;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE)
        mMouseLeft = true;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE)
        mMouseRight = true;

    // Double [MMB] (no mods) -> reset view
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action != GLFW_RELEASE && clickCount > 1 && mods == 0)
        onResetView();

    // Double [LMB] (no mods) -> center on pos
    if (mUseDefaultCameraHandlingLeft && button == GLFW_MOUSE_BUTTON_LEFT && action != GLFW_RELEASE && clickCount > 1 && mods == 0)
    {
        glm::vec3 pos;
        float depth;
        if (mUseRenderingPipeline && mUseDefaultCameraHandling && mPipeline->queryPosition3D(x, y, &pos, &depth))
        {
            mCamera->setPosition(pos - mCamera->getForwardDirection() * mCamera->getLookAtDistance());
            mCamera->setTarget(pos, {0, 1, 0});
        }
    }

    return false;
}

bool GlfwApp::onMouseScroll(double sx, double sy)
{
    if (TwEventMouseWheelGLFW(mWindow, sx, sy))
        return true;

    // camera handling
    if (mCamera && mUseDefaultCameraHandling && sy != 0)
    {
        auto f = glm::pow(1 + mCameraScrollSpeed / 100.0, -sy);
        float camDis = mCamera->getLookAtDistance() * f;
        camDis = glm::clamp(camDis, mCamera->getNearClippingPlane() * 2, mCamera->getFarClippingPlane() / 2);

        // update target AND lookAtDis
        mCamera->setPosition(mCamera->getTarget() - mCamera->getForwardDirection() * camDis);
        mCamera->setLookAtDistance(camDis);
    }

    return false;
}

bool GlfwApp::onMouseEnter()
{
    return false;
}

bool GlfwApp::onMouseExit()
{
    return false;
}

bool GlfwApp::onFocusGain()
{
    return false;
}

bool GlfwApp::onFocusLost()
{
    return false;
}

bool GlfwApp::onFileDrop(const std::vector<std::string> &files)
{
    return false;
}

void GlfwApp::onResetView()
{
    mCamera->setPosition(-mCamera->getForwardDirection() * mCamera->getLookAtDistance());
    mCamera->setTarget({0, 0, 0}, {0, 1, 0});
}

void GlfwApp::mainLoop()
{
    // Loop until the user closes the window
    int frames = 0;
    double lastTime = glfwGetTime();
    double lastStatsTime = lastTime;
    double timeAccum = 0.000001;
    mCurrentTime = 0.0;
    size_t primitives = 0;
    size_t fragments = 0;
    double gpuTime = 0;
    double cpuTime = 0;
    int updatesPerFrame = 1;
    double renderTimestep = 1.0 / mUpdateRate; // tracks time between renders, init is for first frame only
    while (!shouldClose())
    {
        updateInput();

        // Update
        {
            double dt = 1.0 / mUpdateRate;

            // # of updates
            auto updates = updatesPerFrame;
            if (timeAccum > updatesPerFrame * dt) // lags one behind: do one more
                ++updates;
            if (timeAccum < -dt) // is more than one ahead: skip one
                --updates;

            // do updates
            for (auto i = 0; i < updates; ++i)
            {
                auto cpuStart = glfwGetTime();
                update(dt);
                cpuTime += glfwGetTime() - cpuStart;
                timeAccum -= dt;
                mCurrentTime += dt;
            }

            // update adjustment (AFTER updates! timeAccum should be within -dt..dt now)
            if (timeAccum > 2.5 * dt)
            {
                ++updatesPerFrame;
                // glow::info() << "increasing frames per sec";
            }
            else if (timeAccum < -2.5 * dt)
            {
                if (updatesPerFrame > 0)
                    --updatesPerFrame;
                // glow::info() << "decreasing frames per sec";
            }

            // frameskip
            if (timeAccum > mMaxFrameSkip * dt)
            {
                glow::warning() << "Too many updates queued, frame skip of " << timeAccum << " secs";
                timeAccum = mMaxFrameSkip * dt * 0.5;
            }

            // glow::info() << updates << ", " << timeAccum / dt;
        }

        beginRender();

        // Render here
        {
            if (mQueryStats && mPrimitiveQuery)
            {
                // assert(mPrimitiveQuery && "did you forgot to call GlfwApp::init() in your init?");
                mPrimitiveQuery->begin();
                mOcclusionQuery->begin();
                mRenderStartQuery->saveTimestamp();
            }

            render(renderTimestep);

            if (mQueryStats && mPrimitiveQuery)
            {
                mPrimitiveQuery->end();
                mOcclusionQuery->end();
                mRenderEndQuery->saveTimestamp();
            }
        }

        endRender();

        // timing
        auto now = glfwGetTime();
        renderTimestep = now - lastTime;
        timeAccum += now - lastTime;
        lastTime = now;
        ++frames;

        if (mQueryStats && mPrimitiveQuery)
        {
            primitives += mPrimitiveQuery->getResult64();
            fragments += mOcclusionQuery->getResult64();
            gpuTime += TimerQuery::toSeconds(mRenderEndQuery->getSavedTimestamp() - mRenderStartQuery->getSavedTimestamp());
        }

        if (mOutputStatsInterval > 0 && lastTime > lastStatsTime + mOutputStatsInterval)
        {
            double fps = frames / (lastTime - lastStatsTime);
            if (mQueryStats)
                glow::info() << fmt::format("FPS: {:.1f}, CPU: {:.1f} ms, GPU: {:.1f} ms, Primitives: {}, Frags: {}",
                                            fps, cpuTime / frames * 1000., gpuTime / frames * 1000.,
                                            thousandSep(primitives / frames), thousandSep(fragments / frames));
            else
                glow::info() << fmt::format("FPS: {:.1f}, CPU: {:.1f} ms", fps, cpuTime / frames * 1000.);
            lastStatsTime = lastTime;
            frames = 0;
            primitives = 0;
            fragments = 0;
            gpuTime = 0;
            cpuTime = 0;
        }
    }
}

void GlfwApp::internalOnMouseButton(double x, double y, int button, int action, int mods)
{
    // check double click
    if (distance(mClickPos, glm::vec2(x, y)) > 5) // too far
        mClickCount = 0;
    if (mClickTimer.getTimeDiffInSecondsD() > mDoubleClickTime) // too slow
        mClickCount = 0;
    if (mClickButton != button) // wrong button
        mClickCount = 0;

    mClickTimer.restart();
    mClickButton = button;
    mClickPos = {x, y};
    mClickCount++;

    onMouseButton(x, y, button, action, mods, mClickCount);
}

void GlfwApp::updateInput()
{
    // update cursor mode
    switch (mCursorMode)
    {
    case CursorMode::Normal:
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case CursorMode::Hidden:
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        break;
    case CursorMode::Disabled:
        glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    }

    // Poll for and process events
    glfwPollEvents();
}

void GlfwApp::beginRender()
{
    // vsync
    glfwSwapInterval(mVSync ? 1 : 0);

    // viewport
    glViewport(0, 0, mWindowWidth, mWindowHeight);
}

void GlfwApp::endRender()
{
    // draw the tweak bar(s)
    if (mDrawTweakbars)
    {
        TwDraw();

        // unbind TweakBar stuff
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glBindVertexArray(0);
    }

    // Swap front and back buffers
    glfwSwapBuffers(mWindow);
}

void GlfwApp::sleepSeconds(double seconds) const
{
    if (seconds <= 0.0)
        return;

    std::this_thread::sleep_for(std::chrono::microseconds((int64_t)(seconds * 1000 * 1000)));
}

int GlfwApp::run(int argc, char *argv[])
{
    static GlfwApp *currApp = nullptr;
    assert(currApp == nullptr && "cannot run multiple apps simulatenously");
    currApp = this;

    assert(mWindow == nullptr);
    assert(mTweakbar == nullptr);

    // Taken from http://www.glfw.org/documentation.html
    // Initialize the library
    if (!glfwInit())
    {
        std::cerr << "Unable to initialize GLFW" << std::endl;
        return -1;
    }

    // Request debug context
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Try to get core context 4.5
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

    // Create a windowed mode window and its OpenGL context
    mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle.c_str(), NULL, NULL);
    if (!mWindow)
    {
        std::cerr << "Unable to get OpenGL 4.5 Core Debug Context. Trying again with Compat." << std::endl;
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle.c_str(), NULL, NULL);

        if (!mWindow)
        {
            std::cerr << "Unable to create a GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
    }

    // Make the window's context current
    glfwMakeContextCurrent(mWindow);

    // WORKAROUND for Intel bug (3.3 available but 3.0 returned UNLESS explicitly requested)
#define GL_CALL
#ifdef GLOW_COMPILER_MSVC
#if _WIN32
#undef GL_CALL
#define GL_CALL __stdcall // 32bit windows needs a special calling convention
#endif
#endif
    using glGetIntegerFunc = void GL_CALL(GLenum, GLint *);
    auto getGlInt = (glGetIntegerFunc *)glfwGetProcAddress("glGetIntegerv");
    GLint gmajor, gminor;
    getGlInt(GL_MAJOR_VERSION, &gmajor);
    getGlInt(GL_MINOR_VERSION, &gminor);
    if (gmajor * 10 + gminor < 33)
    {
        // destroy current window
        glfwDestroyWindow(mWindow);

        // request vanilla 3.3 context
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_FALSE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        // re-try window creation
        mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, mTitle.c_str(), NULL, NULL);
        if (!mWindow)
        {
            std::cerr << "Unable to create a GLFW window with OpenGL 3.3. (GLOW requires at least 3.3)" << std::endl;
            glfwTerminate();
            return -1;
        }

        // Make the window's context current (again)
        glfwMakeContextCurrent(mWindow);
    }

    // Initialize GLOW
    if (!glow::initGLOW())
    {
        std::cerr << "Unable to initialize GLOW" << std::endl;
        return -1;
    }

    // restore ogl state
    glow::restoreDefaultOpenGLState();

    // anttweakbar
    TwInit(TW_OPENGL_CORE, NULL); // for core profile
    TwWindowSize(mWindowWidth, mWindowHeight);
    mTweakbar = TwNewBar("Tweakbar");

    // unbind any ogl object (especially from AntTweakBar)
    glow::unbindOpenGLObjects();

    // input callbacks
    {
        glfwSetKeyCallback(mWindow, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
            currApp->onKey(key, scancode, action, mods);
        });
        glfwSetCharModsCallback(
            mWindow, [](GLFWwindow *win, unsigned int codepoint, int mods) { currApp->onChar(codepoint, mods); });
        glfwSetMouseButtonCallback(mWindow, [](GLFWwindow *win, int button, int action, int mods) {
            currApp->internalOnMouseButton(currApp->mMouseX, currApp->mMouseY, button, action, mods);
        });
        glfwSetCursorEnterCallback(mWindow, [](GLFWwindow *win, int entered) {
            if (entered)
                currApp->onMouseEnter();
            else
                currApp->onMouseExit();
        });

        glfwSetCursorPosCallback(mWindow, [](GLFWwindow *win, double x, double y) {
            currApp->mMouseX = x;
            currApp->mMouseY = y;
            currApp->onMousePosition(x, y);
        });
        glfwSetScrollCallback(mWindow, [](GLFWwindow *win, double sx, double sy) { currApp->onMouseScroll(sx, sy); });
        glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *win, int w, int h) {
            currApp->mWindowWidth = w;
            currApp->mWindowHeight = h;

            currApp->onResize(w, h);
            TwWindowSize(w, h);
        });
        glfwSetWindowFocusCallback(mWindow, [](GLFWwindow *win, int focused) {
            if (focused)
                currApp->onFocusGain();
            else
                currApp->onFocusLost();
        });
        glfwSetDropCallback(mWindow, [](GLFWwindow *win, int count, const char **paths) {
            std::vector<std::string> files;
            for (auto i = 0; i < count; ++i)
                files.push_back(paths[i]);
            currApp->onFileDrop(files);
        });
    }

    // init app
    init();

    glfwGetFramebufferSize(mWindow, &mWindowWidth, &mWindowHeight);
    onResize(mWindowWidth, mWindowHeight);

    // Execute main loop
    mainLoop();

    // cleanup
    {
        onClose();

        TwTerminate();
        glfwTerminate();

        // aion dump
        if (mDumpTimingsOnShutdown)
            aion::ActionAnalyzer::dumpSummary(std::cout, false);
    }

    return 0;
}

GlfwApp::~GlfwApp()
{
    // empty
}
