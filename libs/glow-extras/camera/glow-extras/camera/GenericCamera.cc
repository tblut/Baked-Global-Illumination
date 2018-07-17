#include "GenericCamera.hh"

#include <glow/common/log.hh>

#include <cassert>

#include <glm/ext.hpp>

#ifdef GLOW_DEBUG
namespace
{
bool isApproxEqual(const glm::mat4 &_v1, const glm::mat4 &_v2, float _eps = .01)
{
    glm::mat4 diff = _v1 - _v2;
    float d = 0;
    d += glm::abs(diff[0][0]);
    d += glm::abs(diff[0][1]);
    d += glm::abs(diff[0][2]);
    d += glm::abs(diff[0][3]);
    d += glm::abs(diff[1][0]);
    d += glm::abs(diff[1][1]);
    d += glm::abs(diff[1][2]);
    d += glm::abs(diff[1][3]);
    d += glm::abs(diff[2][0]);
    d += glm::abs(diff[2][1]);
    d += glm::abs(diff[2][2]);
    d += glm::abs(diff[2][3]);
    d += glm::abs(diff[3][0]);
    d += glm::abs(diff[3][1]);
    d += glm::abs(diff[3][2]);
    d += glm::abs(diff[3][3]);
    return d < _eps;
}

bool isApproxEqual(const glm::mat3 &_v1, const glm::mat3 &_v2, float _eps = .01)
{
    glm::mat3 diff = _v1 - _v2;
    float d = 0;
    d += glm::abs(diff[0][0]);
    d += glm::abs(diff[0][1]);
    d += glm::abs(diff[0][2]);
    d += glm::abs(diff[1][0]);
    d += glm::abs(diff[1][1]);
    d += glm::abs(diff[1][2]);
    d += glm::abs(diff[2][0]);
    d += glm::abs(diff[2][1]);
    d += glm::abs(diff[2][2]);
    return d < _eps;
}

bool isOrthonormalMatrix(const glm::mat3 &_matrix)
{
    return isApproxEqual(glm::inverse(_matrix), glm::transpose(_matrix));
}
}
#endif

using namespace glow::camera;
using namespace std;

GenericCamera::GenericCamera()
{
    setRotationMatrix(glm::mat3(1.0f));
}

GenericCamera::GenericCamera(const std::string &_state)
{
    setStateFromString(_state);
}

void GenericCamera::FPSstyleLookAround(float _deltaX, float _deltaY)
{
    auto fwd = getForwardDirection();

    auto altitude = glm::atan(fwd.y, length(glm::vec2(fwd.x, fwd.z)));
    auto azimuth = glm::atan(fwd.z, fwd.x);

    azimuth += _deltaX;
    altitude = glm::clamp(altitude - _deltaY, -0.499f * glm::pi<float>(), 0.499f * glm::pi<float>());

    auto caz = glm::cos(azimuth);
    auto saz = glm::sin(azimuth);
    auto cal = glm::cos(altitude);
    auto sal = glm::sin(altitude);

    fwd = glm::vec3( //
        cal * caz,   //
        sal,         //
        cal * saz    //
    );
    auto right = normalize(cross(fwd, glm::vec3(0, 1, 0)));
    auto up = cross(right, fwd);

    setRotationMatrix(transpose(glm::mat3(right, up, -fwd)));
}

void GenericCamera::rotateAroundTaget_GlobalAxes(float _x, float _y, float _z)
{
    // move camera so, that the target is the center, then rotate around the
    // global coordinate system

    glm::mat3 t = glm::mat3(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    rotateAroundTaget_helper(_x, _y, _z, t);
}

void GenericCamera::rotateAroundTaget_LocalAxes(float _x, float _y, float _z)
{
    glm::mat3 R = getRotationMatrix3();
    R = glm::transpose(R);

    rotateAroundTaget_helper(_x, _y, _z, R);
}

void GenericCamera::rotateAroundTaget_helper(float _x, float _y, float _z, const glm::mat3 &_rotationAxes)
{
    glm::vec4 T = glm::vec4(getTarget(), 1.0f);
    glm::vec4 P = glm::vec4(getPosition(), 1.0f);

    glm::vec4 tempPos = P - T;
    glm::mat4 newRotation = glm::rotate(glm::mat4(), _x, _rotationAxes[0]);
    newRotation = glm::rotate(newRotation, _y, _rotationAxes[1]);
    newRotation = glm::rotate(newRotation, _z, _rotationAxes[2]);

    tempPos = newRotation * tempPos;

    P = tempPos + T; // new position
    glm::vec4 N = glm::vec4(getUpDirection(), 1.0f);
    N = newRotation * N;

    setLookAtMatrix(glm::vec3(P), glm::vec3(T), glm::vec3(N));
}


void GenericCamera::setHorizontalFieldOfView(float _fovh)
{
    assert(_fovh < 180.0f);
    assert(_fovh > 0.0f);
    mHorizontalFieldOfView = _fovh;
}

void GenericCamera::setVerticalFieldOfView(float _fovv)
{
    assert(_fovv < 180.0f);
    assert(_fovv > 0.0f);

    // we only save the aspectRatio and the horizontal FoV
    // so if we change the vertical FoV, we change the aspectRatio

    // mAspectRatio = tan( glm::radians(0.5f * mHorizontalFieldOfView) ) / tan( glm::radians(0.5f * _fovv) );

    float x = tan(glm::radians(0.5f * _fovv)) * mAspectRatio;
    mHorizontalFieldOfView = glm::degrees(2.0f * atan(x));
}

float GenericCamera::getVerticalFieldOfView() const
{
    return glm::degrees(atan(tan(glm::radians(0.5f * mHorizontalFieldOfView)) / mAspectRatio) * 2.0f);
}

void GenericCamera::setNearClippingPlane(float _plane)
{
    assert(_plane > 0.0f);
    mNearClippingPlane = _plane;
}

void GenericCamera::setFarClippingPlane(float _plane)
{
    assert(_plane > 0.0f);
    mFarClippingPlane = _plane;
}

glm::mat4 GenericCamera::getViewMatrix() const
{
    if (mStereoMode == Mono)
    {
        return getMonoViewMatrix();
    }
    else
    {
        // all kinds of stereo
        bool eyeIsLeftEye = (getEye() == EyeLeft);
        return getStereoViewMatrix(eyeIsLeftEye, mStereoMode);
    }
}

glm::mat4 GenericCamera::getStereoViewMatrix(bool _leftEye, StereoMode _stereoMode) const
{
    // The view matrix is independent of the projection mode (isometric or perspective)
    // so only the stereo mode has to be checked.
    assert(_stereoMode != Mono && "mono is not a stereo mode!");

    float cameraPositionShiftValue = (mInterpupillaryDistance * 0.5f); // shift to the right
    if (_leftEye)
        cameraPositionShiftValue *= -1.0f; // if left eye shift to the left

    if ((_stereoMode == ParallelShift) || (_stereoMode == OffAxis))
    {
        //
        // parallel shift and off-axis have the same view matrices:
        // just shift the camera position to the left/right by half the eye-distance
        //

        // ACGL::Utils::debug() << "WARNING: getStereoViewMatrix() is not tested yet" << std::endl; // remove after
        // testing

        glm::mat3 inverseRotation = getInverseRotationMatrix3();
        glm::vec3 eyePosition = mPosition + (inverseRotation * glm::vec3(cameraPositionShiftValue, 0.0f, 0.0f));

        glm::mat4 m(mRotationMatrix);
        m[3][0] = -(m[0][0] * eyePosition.x + m[1][0] * eyePosition.y + m[2][0] * eyePosition.z);
        m[3][1] = -(m[0][1] * eyePosition.x + m[1][1] * eyePosition.y + m[2][1] * eyePosition.z);
        m[3][2] = -(m[0][2] * eyePosition.x + m[1][2] * eyePosition.y + m[2][2] * eyePosition.z);
        return m;
    }

    // else it has to be toe-in:
    assert(_stereoMode == ToeIn && "unsupported stereo mode!");
    //
    // Toe-in: shift the camera position to the left/right by half the eye-distance and
    //         rotate a bit inwards so that the two cameras focus the same point
    //         at the look-at distance (focal point)

    assert(0 && "getStereoViewMatrix() for TOE_IN is not implemented yet!");
    return glm::mat4();
}

glm::mat4 GenericCamera::getInverseViewMatrix() const
{
    if (mStereoMode == Mono)
    {
        return getMonoInverseViewMatrix();
    }

    glm::mat4 viewMatrix = getViewMatrix();
    return glm::inverse(viewMatrix);
}

glm::mat4 GenericCamera::getProjectionMatrix() const
{
    if (mStereoMode == Mono)
    {
        return getMonoProjectionMatrix();
    }
    else
    {
        // all kinds of stereo
        bool eyeIsLeftEye = (getEye() == EyeLeft);
        return getStereoProjectionMatrix(eyeIsLeftEye, mStereoMode);
    }
}

glm::mat4 GenericCamera::getMonoProjectionMatrix() const
{
    glm::mat4 projectionMatrix; // identity matrix

    if (getProjectionMode() == IsometricProjection)
    {
        // we don't set the left/right/top/bottom values explicitly, so we want that
        // all object at our focal distance appear the same in perspective and isometric view
        float right = tan(glm::radians(getHorizontalFieldOfView() * 0.5f)) * mLookAtDistance;
        float left = -right;
        float top = tan(glm::radians(getVerticalFieldOfView() * 0.5f)) * mLookAtDistance;
        float bottom = -top;

        // we do the same here as a glOrtho call would do.
        projectionMatrix[0][0] = 2.0f / (right - left);
        projectionMatrix[1][1] = 2.0f / (top - bottom);
        projectionMatrix[2][2] = -2.0f / (mFarClippingPlane - mNearClippingPlane);
        projectionMatrix[0][3] = -(right + left) / (right - left);
        projectionMatrix[1][3] = -(top + bottom) / (top - bottom);
        projectionMatrix[2][3] = -(mFarClippingPlane + mNearClippingPlane) / (mFarClippingPlane - mNearClippingPlane);
        projectionMatrix[3][3] = 1.0;
    }
    else if (mProjectionMode == PerspectiveProjectionOpenGL)
    {
        if (std::isinf(mFarClippingPlane))
        {
            float e = 1.0f / tan(glm::radians(getVerticalFieldOfView() * 0.5f));
            const float a = getAspectRatio();

            // infinite Perspective matrix reversed mapping to 1..-1
            projectionMatrix = {e / a, 0.0f,  0.0f, 0.0f, 0.0f,
                                e,     0.0f,  0.0f, 0.0f, 0.0f,
                                -1.0f, -1.0f, 0.0f, 0.0f, -2.0 * mNearClippingPlane,
                                0.0f};
        }
        else
        {
            projectionMatrix = glm::perspective(glm::radians((float)getHorizontalFieldOfView()), (float)getAspectRatio(),
                                                (float)mNearClippingPlane, (float)mFarClippingPlane);
        }
    }
    else if (mProjectionMode == PerspectiveProjectionDXReverse)
    {
        if (std::isinf(mFarClippingPlane))
        {
            float e = 1.0f / tan(glm::radians(getVerticalFieldOfView() * 0.5f));
            const float a = getAspectRatio();

            // infinite Perspective matrix reversed mapping to 1..0
            projectionMatrix = {
                e / a, 0.0f, 0.0f, 0.0f, 0.0f, e, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, mNearClippingPlane,
                0.0f};
        }
        else
        {
            assert(0 && "unsupported projection mode");
        }
    }

    else
        assert(0 && "unsupported projection mode");

    return projectionMatrix;
}

glm::mat4 GenericCamera::getMonoViewMatrix() const
{
    glm::mat4 m(mRotationMatrix);
    m[3][0] = -(m[0][0] * mPosition.x + m[1][0] * mPosition.y + m[2][0] * mPosition.z);
    m[3][1] = -(m[0][1] * mPosition.x + m[1][1] * mPosition.y + m[2][1] * mPosition.z);
    m[3][2] = -(m[0][2] * mPosition.x + m[1][2] * mPosition.y + m[2][2] * mPosition.z);
    assert(isApproxEqual(getRotationMatrix4() * getTranslationMatrix4(), m));
    return m;
}

glm::mat4 GenericCamera::getMonoInverseViewMatrix() const
{
    glm::mat4 m(glm::transpose(mRotationMatrix));
    m[3][0] = mPosition.x;
    m[3][1] = mPosition.y;
    m[3][2] = mPosition.z;
    assert(isApproxEqual(glm::inverse(getViewMatrix()), m));
    return m;
}

glm::mat4 GenericCamera::getStereoProjectionMatrix(bool _leftEye, StereoMode _stereoMode) const
{
    assert(_stereoMode != Mono && "mono is not a stereo mode!");

    if (getProjectionMode() == IsometricProjection)
    {
        // very unusual, prepare for headaches!
        return getMonoProjectionMatrix();
    }

    if ((_stereoMode == ParallelShift) || (_stereoMode == ToeIn))
    {
        // the view matrix changes but the projection matrix stays the same
        return getMonoProjectionMatrix();
    }

    // so off-axis it is!
    assert(_stereoMode == OffAxis && "unknown projection mode!");
    //
    // In this mode the camera positions (view matrix) is shifted to the left/right still looking
    // straight ahead. The projection is also looking ahead but the projection center is
    // off (hence off-axis).
    // There is one plane in front of the cameras where the view-frusta match.
    // This should be the distance to the physical screen from the users position.


    assert(0 && "getStereoViewMatrix() is not implemented for OFF_AXIS yet!");
    return glm::mat4();
}


/// Writes all internal state to one string
/// Elements are seperated by pipes ('|'), spaces can get ignored.
std::string GenericCamera::storeStateToString() const
{
    error() << "Not implemented";
    return "";
    /*std::string state;

    state = "ACGL_GenericCamera | "; // "magic number", for every version the same
    state += "1 | ";                 // version, always an integer

    state += toString(mPosition) + " | ";
    state += toString(mRotationMatrix) + " | ";
    if (mProjectionMode == ISOMETRIC_PROJECTION)
        state += "ISOMETRIC_PROJECTION | ";
    if (mProjectionMode == PERSPECTIVE_PROJECTION_OPENGL)
        state += "PERSPECTIVE_PROJECTION | ";
    if (mStereoMode == MONO)
        state += "MONO | ";
    if (mStereoMode == PARALLEL_SHIFT)
        state += "PARALLEL_SHIFT | ";
    if (mStereoMode == OFF_AXIS)
        state += "OFF_AXIS | ";
    if (mStereoMode == TOE_IN)
        state += "TOE_IN | ";
    if (mCurrentEye == EYE_LEFT)
        state += "EYE_LEFT | ";
    if (mCurrentEye == EYE_RIGHT)
        state += "EYE_RIGHT | ";
    state += toString(mHorizontalFieldOfView) + " | ";
    state += toString(mAspectRatio) + " | ";
    state += toString(mInterpupillaryDistance) + " | ";
    state += toString(mNearClippingPlane) + " | ";
    state += toString(mFarClippingPlane) + " | ";
    state += toString(mLookAtDistance) + " | ";
    state += toString(mViewportSize);

    return state;*/
}

/// Sets all internal state from a string
void GenericCamera::setStateFromString(const std::string &_state)
{
    error() << "Not implemented";
    /*vector<string> token = split(_state, '|');
    for (size_t i = 0; i < token.size(); i++)
    {
        token[i] = stripOfWhiteSpaces(token[i]);
    }
    if ((token.size() < 14) || (token[0] != "ACGL_GenericCamera"))
    {
        ACGL::Utils::error() << "Generic camera state string is invalid: " << _state << std::endl;
        return;
    }
    if (to<int>(token[1]) != 1)
    {
        ACGL::Utils::error() << "Generic camera state version not supported: " << to<int>(token[1]) << std::endl;
        return;
    }

    int pos = 2;
    mPosition = toVec3(token[pos++]);
    mRotationMatrix = toMat3(token[pos++]);
    if (token[pos] == "ISOMETRIC_PROJECTION")
        mProjectionMode = ISOMETRIC_PROJECTION;
    if (token[pos] == "PERSPECTIVE_PROJECTION")
        mProjectionMode = PERSPECTIVE_PROJECTION_OPENGL;
    pos++;
    if (token[pos] == "MONO")
        mStereoMode = MONO;
    if (token[pos] == "PARALLEL_SHIFT")
        mStereoMode = PARALLEL_SHIFT;
    if (token[pos] == "OFF_AXIS")
        mStereoMode = OFF_AXIS;
    if (token[pos] == "TOE_IN")
        mStereoMode = TOE_IN;
    pos++;
    if (token[pos] == "EYE_LEFT")
        mCurrentEye = EYE_LEFT;
    if (token[pos] == "EYE_RIGHT")
        mCurrentEye = EYE_RIGHT;
    pos++;

    mHorizontalFieldOfView = to<float>(token[pos++]);
    mAspectRatio = to<float>(token[pos++]);
    mInterpupillaryDistance = to<float>(token[pos++]);
    mNearClippingPlane = to<float>(token[pos++]);
    mFarClippingPlane = to<float>(token[pos++]);
    mLookAtDistance = to<float>(token[pos++]);
    mViewportSize = toUvec2(token[pos++]);*/
}

float GenericCamera::getFocalLenghtInPixel() const
{
    return ((float)mViewportSize.x) / (2.0f * tan(glm::radians(0.5f * mHorizontalFieldOfView)));
}

void GenericCamera::setFocalLengthInPixel(float _focalLengthInPixel)
{
    float hFoVrad = 2.0f * atan((0.5f * mViewportSize.x) * (1.0f / _focalLengthInPixel));
    setHorizontalFieldOfView(glm::degrees(hFoVrad));
}


void GenericCamera::setRotationMatrix(glm::mat3 _matrix)
{
    mRotationMatrix = _matrix;
    assert(isOrthonormalMatrix(mRotationMatrix));
}

void GenericCamera::setRotationMatrix(glm::mat4 _matrix)
{
    mRotationMatrix = glm::mat3(_matrix);
    assert(isOrthonormalMatrix(mRotationMatrix));
}

void GenericCamera::setLookAtMatrix(const glm::vec3 &_position, const glm::vec3 &_target, const glm::vec3 &_up)
{
    setPosition(_position);
    setTarget(_target, _up);
}

glm::mat4 GenericCamera::getTranslationMatrix4() const
{
    glm::mat4 trans;
    trans[3][0] = -mPosition.x;
    trans[3][1] = -mPosition.y;
    trans[3][2] = -mPosition.z;
    return trans;
}

glm::vec3 GenericCamera::getUpDirection() const
{
    glm::vec3 up(mRotationMatrix[0][1], mRotationMatrix[1][1], mRotationMatrix[2][1]);
    assert(glm::distance(getInverseRotationMatrix3() * glm::vec3(0.0f, 1.0f, 0.0f), up) < .01);
    return up;
}
glm::vec3 GenericCamera::getRightDirection() const
{
    glm::vec3 right(mRotationMatrix[0][0], mRotationMatrix[1][0], mRotationMatrix[2][0]);
    assert(glm::distance(getInverseRotationMatrix3() * glm::vec3(1.0f, 0.0f, 0.0f), right) < .01);
    return right;
}
glm::vec3 GenericCamera::getForwardDirection() const
{
    glm::vec3 forward(-mRotationMatrix[0][2], -mRotationMatrix[1][2], -mRotationMatrix[2][2]);
    assert(glm::distance(getInverseRotationMatrix3() * glm::vec3(0.0f, 0.0f, -1.0f), forward) < .01);
    return forward;
}

void GenericCamera::getViewRay(glm::vec2 mousePosition, glm::vec3 *pos, glm::vec3 *dir) const
{
    glm::vec3 ps[2];
    auto i = 0;
    for (auto d : {0.5f, -0.5f})
    {
        glm::vec4 v{mousePosition.x * 2 - 1, 1 - mousePosition.y * 2, d * 2 - 1, 1.0};

        v = glm::inverse(getProjectionMatrix()) * v;
        v /= v.w;
        v = glm::inverse(getViewMatrix()) * v;
        ps[i++] = glm::vec3(v);
    }

    if (pos)
        *pos = getPosition();
    if (dir)
        *dir = normalize(ps[0] - ps[1]);
}

void GenericCamera::setTarget(const glm::vec3 &_target, const glm::vec3 &_up)
{
    glm::vec3 forwardVector = _target - mPosition;
    mLookAtDistance = glm::length(forwardVector);
    if (mLookAtDistance < .0001) // in case target == mPosition
    {
        mLookAtDistance = .0001;
        forwardVector = glm::vec3(mLookAtDistance, 0, 0);
    }

    forwardVector = forwardVector / (float)mLookAtDistance; // normalize
    glm::vec3 rightVector = glm::normalize(glm::cross(forwardVector, _up));
    glm::vec3 upVector = glm::cross(rightVector, forwardVector);

    glm::mat3 rotMatrix;
    rotMatrix[0][0] = rightVector.x;
    rotMatrix[0][1] = upVector.x;
    rotMatrix[0][2] = -forwardVector.x;
    rotMatrix[1][0] = rightVector.y;
    rotMatrix[1][1] = upVector.y;
    rotMatrix[1][2] = -forwardVector.y;
    rotMatrix[2][0] = rightVector.z;
    rotMatrix[2][1] = upVector.z;
    rotMatrix[2][2] = -forwardVector.z;

    setRotationMatrix(rotMatrix);
}

void GenericCamera::setLookAtDistance(float _distance)
{
    assert(_distance > 0.0f);
    mLookAtDistance = _distance;
}

glm::mat4 GenericCamera::getModelMatrix() const
{
    glm::mat4 m(mRotationMatrix);
    m[3][0] = -(m[0][0] * mPosition.x + m[1][0] * mPosition.y + m[2][0] * mPosition.z);
    m[3][1] = -(m[0][1] * mPosition.x + m[1][1] * mPosition.y + m[2][1] * mPosition.z);
    m[3][2] = -(m[0][2] * mPosition.x + m[1][2] * mPosition.y + m[2][2] * mPosition.z);
    assert(isApproxEqual(getRotationMatrix4() * getTranslationMatrix4(), m));
    return m;
}


glm::mat4 GenericCamera::getInverseModelMatrix() const
{
    glm::mat4 modelMatrix = getModelMatrix();
    return glm::inverse(modelMatrix);
}


void GenericCamera::move(const glm::vec3 &_vector)
{
    glm::mat3 inverseRotation = getInverseRotationMatrix3();
    mPosition += (inverseRotation * _vector);
}
