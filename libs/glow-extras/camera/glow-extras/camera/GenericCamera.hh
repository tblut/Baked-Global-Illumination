#pragma once

#include <glm/gtx/quaternion.hpp>
#include <glm/vec3.hpp>

#include <string>

#include "CameraBase.hh"

/*
 * What you definitly want to set:
 * - a position in 3D space (a vec3)
 * - a viewing direction, this can be defined by:
 *   - roll/pitch/jaw rotations
 *   - up/left/forward vectors
 * - the aspect ratio (width/height)
 *
 * What you maybe want to change:
 * - a lookAtDistance, this is internaly only used for the orthographic
 *   projection, can be be used externaly e.g. for field of view effects
 *   (if no focal distance is given, a default will be used, but often this
 *   value is not used at all), also a lookAt point can be calculated with this
 * - Stereo settings:
 *   - the eyedistance
 *   - the StereoMode
 * - Horizontal/Vertical FoV
 * - near- and far-clipping plane
 *
 * A Camera can calculate:
 * - a ViewMatrix
 * - a ProjectionMatrix for Mono view
 * - ProjectionMatrices for Stereo view
 * - etc.
 *
 *
 * Note: To get from world to camera space, the translation is applied first, then the
 *       rotation. getViewMatrix() provides one matrix for this.
 *       Other camera models rotate first and translate later (e.g. bundler)! The rotation
 *       is the same, the translation differs.
 *
 * TODO: support more stereo modes!
 */

namespace glow
{
namespace camera
{
GLOW_SHARED(class, GenericCamera);
class GenericCamera : public CameraBase
{
public:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Helping enums:
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// use the DX reverse mode with:
    /// * an infinite far plane
    /// * a float z-buffer
    /// * glClearDepth(0.0)
    /// * glDepthFunc(GL_GREATER)
    /// * either:
    ///     * glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE) (DX style mapping of the Z values)
    ///   or:
    ///     * glDepthRangedNV(-1.0, 1.0)
    ///
    /// this way near will get mapped to 1.0 and infinite to 0.0
    enum ProjectionMode
    {
        IsometricProjection = 0,
        PerspectiveProjectionOpenGL,   // maps to -1..1
        PerspectiveProjectionDXReverse // maps to  1..0
    };

    enum StereoMode
    {
        Mono = 0,
        ParallelShift,
        OffAxis,
        ToeIn
    };

    enum Eye
    {
        EyeLeft = 0,
        EyeRight
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Constructor / Destructor / save&store
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * Default Constructor of a camera.
     */
    GenericCamera();

    /**
     * Constructs a camera from a string which holds all camera parameters. Note that this sets the viewport which
     * might not fit the correct screen!
     */
    GenericCamera(const std::string &_state);

    /**
     * Destructor of a camera.
     */
    ~GenericCamera() {}
    /// Writes all internal state to one string
    std::string storeStateToString() const;

    /// Sets all internal state from a string
    void setStateFromString(const std::string &_state);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set / Get basic camera properties:
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Set the distance between the pupils (only needed for stereo rendering).
     * @param _interpupillaryDistance Inter pupil distance in meter(!) (distance between the centers of the eyes)
     */
    void setInterpupillaryDistance(float _interpupillaryDistance) { mInterpupillaryDistance = _interpupillaryDistance; }
    float getInterpupillaryDistance() const { return mInterpupillaryDistance; }
    /**
* Set the projection mode of the camera.
* @param _projection           New projection mode of the camera.
     */
    void setProjectionMode(ProjectionMode _projection) { mProjectionMode = _projection; }
    ProjectionMode getProjectionMode() const { return mProjectionMode; }
    /**
     * Set the stereo mode of the camera. In mode MONO the set eye will get ignored (see below).
     * @param _stereoMode New stereo mode of the camera.
     */
    void setStereoMode(StereoMode _stereoMode) { mStereoMode = _stereoMode; }
    StereoMode getStereoMode() const { return mStereoMode; }
    /**
     * Sets the currently active eye. In stereo mode MONO this setting is ignored.
     * In the stereo modes (PARALLEL_SHIFT, OFF_AXIS, TOE_IN) it is used to
     * define the default eye that is assumed for the generic get*Matrix() functions.
     * (Matrices for specific eyes can still get queried directly without setting the
     * eye explicitly before each call).
     */
    void setEye(Eye _eye) { mCurrentEye = _eye; }
    Eye getEye() const { return mCurrentEye; }
    /**
* Set the horizontal field of view of the camera in degree.
* vertical FoV will get (implicitly) changed!
* @param _fovh         New horizontal field of view of the camera.
     */
    void setHorizontalFieldOfView(float _fovh);
    float getHorizontalFieldOfView() const { return mHorizontalFieldOfView; }
    /**
* Set the vertical field of view of the camera in degree.
* Horizontal FoV will get changed!
* @param _fovv         New vertical field of view of the camera.
     */
    void setVerticalFieldOfView(float _fovv);
    float getVerticalFieldOfView() const;

    /**
     * Set the aspect ratio of the camera. The horizontal FoV stays the same, the
     * vertical FoV gets updated.
     * @param aspectRatio New aspect ratio (width/height)
     */
    void setAspectRatio(float _aspectRatio) { mAspectRatio = _aspectRatio; }
    float getAspectRatio() const { return mAspectRatio; }
    /**
     * Set the near clipping plane of the camera.
     * The plane is defined only by a distance from the camera.
     * @param _plane        New near clipping plane of the camera.
     */
    void setNearClippingPlane(float _plane);
    /// Gets the near clip distance
    virtual float getNearClippingPlane() const override { return mNearClippingPlane; }
    /**
     * Set the far clipping plane of the camera.
     * The plane is defined only by a distance from the camera.
     * This distance might be inf!
     * @param _plane        New far clipping plane of the camera.
     */
    void setFarClippingPlane(float _plane);
    /// Gets the far clip distance
    virtual float getFarClippingPlane() const override { return mFarClippingPlane; }
    /// Gets size of the viewport
    virtual glm::uvec2 getViewportSize() const override { return mViewportSize; }
    /// Gets width of the viewport
    unsigned int getViewportWidth() const { return mViewportSize.x; }
    /// Gets height of the viewport
    unsigned int getViewportHeight() const { return mViewportSize.y; }
    /// Sets size of the viewport. NOTE: DOES NOT CHANGE THE ASPECT RATIO! Use resize() if you want to change that as
    /// well!
    void setViewportSize(glm::uvec2 _size) { mViewportSize = _size; }
    /// Sets size of the viewport. NOTE: DOES NOT CHANGE THE ASPECT RATIO! Use resize() if you want to change that as
    /// well!
    void setViewportSize(unsigned int _width, unsigned int _height) { setViewportSize(glm::uvec2(_width, _height)); }
    /// Sets new viewport size and calculates new aspect ratio
    void resize(int _newWidth, int _newHeight)
    {
        setViewportSize(_newWidth, _newHeight);
        setAspectRatio(_newWidth / (float)_newHeight);
    }

    /// The focal length is coupled to the sensor size in real cameras. As this camera does not model a
    /// sensor size in mm, the focal length is given in pixels and is in relation to the viewports resolution.
    /// This model is also used by bundler.
    /// Note that this gives only useful results if the viewports aspect ratio is the same as the
    /// projections aspect ratio!
    float getFocalLenghtInPixel() const;

    /// Sets the focal length in pixel relative to the viewport dimension. This will change the FoV.
    /// See getFocalLenghtInPixel() for more information.
    void setFocalLengthInPixel(float _focalLengthInPixel);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set / Get the matrices:
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Generic view and projection matrices. These obey the set stereo/eye settings.
    // (When in doubt, use these!)
    //

    /// Gets the currently active view matrix (depends on stereo mode and current eye)
    virtual glm::mat4 getViewMatrix() const override;
    /// Gets the currently active view matrix (depends on stereo mode and current eye)
    glm::mat4 getInverseViewMatrix() const;
    /// Gets the currently active projection matrix (depends on stereo mode and current eye)
    virtual glm::mat4 getProjectionMatrix() const override;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Explicit view and projection matrices. These DON'T obey the set stereo/eye settings.
    //

    /// Gets the view matrix for non-stereo rendering EVEN IF A STEREO MODE IS SET!
    glm::mat4 getMonoViewMatrix() const;
    glm::mat4 getMonoInverseViewMatrix() const;

    /**
     * Compute a camera view matrix for stereo rendering.
     * In stereo mode, the view matrix is the mono view matrix but also the shift
     * by half the eye distance to the left/right and a small rotation inwards in
     * case of toe in mode.
     *
     * These methods get the stereo matrix independent of the set mode for this camera.
     */
    glm::mat4 getStereoViewMatrix(bool _leftEye, StereoMode _stereoMode = OffAxis) const;
    glm::mat4 getLeftStereoViewMatrix() const { return getStereoViewMatrix(true, mStereoMode); }
    glm::mat4 getRightStereoViewMatrix() const { return getStereoViewMatrix(false, mStereoMode); }
    glm::mat4 getLeftParallelShiftStereoViewMatrix() const { return getStereoViewMatrix(true, ParallelShift); }
    glm::mat4 getRightParallelShiftStereoViewMatrix() const { return getStereoViewMatrix(false, ParallelShift); }
    glm::mat4 getLeftOffAxisStereoViewMatrix() const { return getStereoViewMatrix(true, OffAxis); }
    glm::mat4 getRightOffAxisStereoViewMatrix() const { return getStereoViewMatrix(false, OffAxis); }
    glm::mat4 getLeftToeInStereoViewMatrix() const { return getStereoViewMatrix(true, ToeIn); }
    glm::mat4 getRightToeInStereoViewMatrix() const { return getStereoViewMatrix(false, ToeIn); }
    /// Gets the projection matrix for non-stereo rendering EVEN IF A STEREO MODE IS SET!
    glm::mat4 getMonoProjectionMatrix() const;

    /**
     * Compute a camera projection matrix for stereo rendering.
     * In stereo mode, the Cameras position is the point in the middle between the two eyes.
     * So we just need one additional info to calculate two matrices:
     */
    glm::mat4 getStereoProjectionMatrix(bool _leftEye, StereoMode _stereoMode = OffAxis) const;
    glm::mat4 getLeftStereoProjectionMatrix() const { return getStereoProjectionMatrix(true, mStereoMode); }
    glm::mat4 getRightStereoProjectionMatrix() const { return getStereoProjectionMatrix(false, mStereoMode); }
    glm::mat4 getLeftParallelShiftStereoProjectionMatrix() const
    {
        return getStereoProjectionMatrix(true, ParallelShift);
    }
    glm::mat4 getRightParallelShiftStereoProjectionMatrix() const
    {
        return getStereoProjectionMatrix(false, ParallelShift);
    }
    glm::mat4 getLeftOffAxisStereoProjectionMatrix() const { return getStereoProjectionMatrix(true, OffAxis); }
    glm::mat4 getRightOffAxisStereoProjectionMatrix() const { return getStereoProjectionMatrix(false, OffAxis); }
    glm::mat4 getLeftToeInStereoProjectionMatrix() const { return getStereoProjectionMatrix(true, ToeIn); }
    glm::mat4 getRightToeInStereoProjectionMatrix() const { return getStereoProjectionMatrix(false, ToeIn); }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set / Get properties that move the camera around (or rotate etc.)
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Look around with a mouse, works like a FPS camera:
     *  No roll possible.
     *  Up/down is limited to 90 degree.
     * This method assumes there is no roll in the current camera rotation, if
     * there is a roll component, it will get destroyed -> don't mix this
     * way to stear with other, more flexible methods!
     * @param _deltaX How much the mouse moved on the viewport (0..1, 1 = full viewport width)
     * @param _deltaY How much the mouse moved on the viewport (0..1, 1 = full viewport height)
     */
    void FPSstyleLookAround(float _deltaX, float _deltaY);

    // rotate around the target, x,y,z are angles to rotate by IN RADIANCE
    // the rotation is around the global goordinate system (1,0,0 / 0,1,0 / 0,0,1)
    // "turntable style" rotation if _x is set and _y,_z == 0
    void rotateAroundTaget_GlobalAxes(float _x, float _y, float _z);

    // rotate around the current coordinate system
    void rotateAroundTaget_LocalAxes(float _x, float _y, float _z);


    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Generic rotation and translation matrices.
    //

    /// Gets the orthonormal rotation matrix (mat3)
    const glm::mat3 &getRotationMatrix3() const { return mRotationMatrix; }
    /// Gets the inverse orthonormal rotation matrix (mat3)
    glm::mat3 getInverseRotationMatrix3() const { return glm::transpose(mRotationMatrix); }
    /// Gets the orthonormal rotation matrix as a mat4
    glm::mat4 getRotationMatrix4() const { return glm::mat4(mRotationMatrix); }
    /// Sets the rotation matrix (mat3)
    void setRotationMatrix(glm::mat3 _matrix);

    /// Sets the rotation matrix (mat3-part of a mat4)
    void setRotationMatrix(glm::mat4 _matrix);

    /// Sets the complete lookat (position and rotation)
    void setLookAtMatrix(const glm::vec3 &_position, const glm::vec3 &_target, const glm::vec3 &_up);

    /// Gets the translation matrix of this object (no rotational element)
    glm::mat4 getTranslationMatrix4() const;

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Generic model matrices.
    //

    /// Gets the currently active view matrix (depends on stereo mode and current eye)
    glm::mat4 getModelMatrix() const;

    /// Gets the currently active view matrix (depends on stereo mode and current eye)
    glm::mat4 getInverseModelMatrix() const;


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set / Get properties that move the object around (or rotate etc.)
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * Set the position of the camera.
     * @param _position          New position of the object.
     */
    void setPosition(const glm::vec3 &_position) { mPosition = _position; }
    glm::vec3 getPosition() const override { return mPosition; }
    /// Moves the object by a given vector (relative to own orientation)
    void move(const glm::vec3 &_vector);
    void moveRight(float _distance) { move(glm::vec3(_distance, 0, 0)); }
    void moveBack(float _distance) { move(glm::vec3(0, 0, _distance)); }
    void moveUp(float _distance) { move(glm::vec3(0, _distance, 0)); }
    void moveLeft(float _distance) { move(glm::vec3(-_distance, 0, 0)); }
    void moveForward(float _distance) { move(glm::vec3(0, 0, -_distance)); }
    void moveDown(float _distance) { move(glm::vec3(0, -_distance, 0)); }
    /**
     * Set the distance of the object to the object it's looking at.
     * Will change the target!
     * @param _distance     New distance of the object this is pointed at.
     */
    void setLookAtDistance(float _distance);
    /// Gets the look-at distance
    float getLookAtDistance() const { return mLookAtDistance; }
    /// Will change the look at distance!
    /// Will change the rotation!
    /// Uses stored up-vector as reference
    /// CAUTION: if you want to set position AND target, ALWAYS set position first!
    void setTarget(const glm::vec3 &_target) { setTarget(_target, getUpDirection()); }
    /// Will change the look at distance!
    /// Will change the rotation!
    /// Uses given up-vector as reference
    /// CAUTION: if you want to set position AND target, ALWAYS set position first!
    void setTarget(const glm::vec3 &_target, const glm::vec3 &_up);

    /// Gets the reconstructed target
    glm::vec3 getTarget() const { return mPosition + getForwardDirection() * getLookAtDistance(); }
    /// Get the unit up direction
    glm::vec3 getUpDirection() const;
    /// Get the unit right direction
    glm::vec3 getRightDirection() const;
    /// Get the unit forward direction
    glm::vec3 getForwardDirection() const;

    /// Returns the world space ray (pos -> dir) for a given mouse position
    /// Mouse position must be in (0,0) .. (1,1)
    /// Note that mousePosition (0,0) is top-left (and not bottom-left)
    void getViewRay(glm::vec2 mousePosition, glm::vec3* pos, glm::vec3* dir) const;

private:
    ///
    /// States: update the storeStateToString() & setStateFromString() functions whenever adding a new state!
    ///

    /// Current camera projection mode
    ProjectionMode mProjectionMode = PerspectiveProjectionOpenGL;
    /// stereo view mode
    StereoMode mStereoMode = Mono;
    /// Current eye
    Eye mCurrentEye = EyeLeft;
    /// Current camera horizontal field of view
    float mHorizontalFieldOfView = 75.0f;
    /// Current aspect ratio: width/height.
    float mAspectRatio = 4.f / 3.f;

    /// Distance of the eyes for stereo projection. In that case, the left eye is 0.5*InterpupillaryDistance
    /// shifted to the left of position and the right eye is 0.5*InterpupillaryDistance to the right shifted.
    /// We assume that 1 unit equals 1 meter. The mean eye distance is 6.5 cm == 0.065 units
    float mInterpupillaryDistance = 0.064f; // 0.064 m = 6.4 cm - mean human eye distance: 6.47cm (male), 6.23cm (female)

    /// Current camera near clipping plane
    float mNearClippingPlane = .1f; // 10cm
    /// Current camera far clipping plane
    float mFarClippingPlane = 5000.; // 5000m

    /// viewport in pixel:
    glm::uvec2 mViewportSize;

    /// Current position
    glm::vec3 mPosition;

    /// Current rotation
    glm::mat3 mRotationMatrix;

    /// might be used later to rotate around this position:
    float mLookAtDistance = 500.0;

    // helper:
    void rotateAroundTaget_helper(float _x, float _y, float _z, const glm::mat3 &_rotationAxes);
};
}
}
