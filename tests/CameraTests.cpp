// Note: DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN is defined in InputTests.cpp
// We don't define it here since we're combining multiple test files
#include "doctest.h"
#include "TestHelpers.h"
#include <glm/gtc/matrix_transform.hpp>

#include "cameras/Camera.h"

// ===================================================================
// CONSTRUCTOR TESTS
// ===================================================================

TEST_CASE("Camera - Constructor initializes with correct default values")
{
    Camera camera{};

    // Check initial position
    glm::vec3 position = camera.GetPosition();
    CHECK(position.x == 0.0f);
    CHECK(position.y == 0.0f);
    CHECK(position.z == 0.0f);

    // Check initial pitch and yaw
    CHECK(FloatApproxEquals(camera.m_pitch, 0.0f));
    CHECK(FloatApproxEquals(camera.m_yaw, -90.0f)); // Default yaw is -90 degrees

    // Check initial front direction (should point along negative Z axis)
    glm::vec3 front = camera.GetFront();
    CHECK(FloatApproxEquals(front.x, 0.0f));
    CHECK(FloatApproxEquals(front.y, 0.0f));
    CHECK(FloatApproxEquals(front.z, -1.0f));

    // Check initial up vector
    glm::vec3 up = camera.GetUp();
    CHECK(Vec3ApproxEquals(up, glm::vec3(0.0f, 1.0f, 0.0f)));
}

// ===================================================================
// UpdateFront TESTS
// ===================================================================

TEST_CASE("UpdateFront - Updates yaw correctly with horizontal mouse movement")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;

    // Move mouse to the right (positive xOffset)
    camera.UpdateFront(10.0f, 0.0f);

    CHECK(FloatApproxEquals(camera.m_yaw, initialYaw + 10.0f));
    CHECK(FloatApproxEquals(camera.m_pitch, 0.0f)); // Pitch should remain unchanged
}

TEST_CASE("UpdateFront - Updates pitch correctly with vertical mouse movement")
{
    Camera camera{};

    float initialPitch = camera.m_pitch;

    // Move mouse up (positive yOffset)
    camera.UpdateFront(0.0f, 5.0f);

    CHECK(FloatApproxEquals(camera.m_pitch, initialPitch + 5.0f));
    CHECK(FloatApproxEquals(camera.m_yaw, -90.0f)); // Yaw should remain unchanged
}

TEST_CASE("UpdateFront - Clamps pitch to maximum range")
{
    Camera camera{};

    // Try to exceed maximum pitch
    camera.UpdateFront(0.0f, 100.0f);

    CHECK(FloatApproxEquals(camera.m_pitch, camera.m_maxRange));
}

TEST_CASE("UpdateFront - Clamps pitch to minimum range")
{
    Camera camera{};

    // Try to exceed minimum pitch
    camera.UpdateFront(0.0f, -100.0f);

    CHECK(FloatApproxEquals(camera.m_pitch, camera.m_minRange));
}

TEST_CASE("UpdateFront - Clamps pitch when already at maximum")
{
    Camera camera{};

    // Set pitch to maximum
    camera.UpdateFront(0.0f, 89.0f);
    CHECK(FloatApproxEquals(camera.m_pitch, 89.0f));

    // Try to increase further
    camera.UpdateFront(0.0f, 10.0f);
    CHECK(FloatApproxEquals(camera.m_pitch, 89.0f)); // Should remain clamped
}

TEST_CASE("UpdateFront - Clamps pitch when already at minimum")
{
    Camera camera{};

    // Set pitch to minimum
    camera.UpdateFront(0.0f, -89.0f);
    CHECK(FloatApproxEquals(camera.m_pitch, -89.0f));

    // Try to decrease further
    camera.UpdateFront(0.0f, -10.0f);
    CHECK(FloatApproxEquals(camera.m_pitch, -89.0f)); // Should remain clamped
}

TEST_CASE("UpdateFront - Updates front direction vector correctly")
{
    Camera camera{};

    // Rotate camera 90 degrees to the right (around Y axis)
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 front = camera.GetFront();
    // After rotating 90 degrees from -90 degrees, yaw should be 0
    // Front should point along positive X axis
    CHECK(FloatApproxEquals(front.x, 1.0f));
    CHECK(FloatApproxEquals(front.y, 0.0f));
    CHECK(FloatApproxEquals(front.z, 0.0f));
}

TEST_CASE("UpdateFront - Front vector is normalized")
{
    Camera camera{};

    // Apply some rotation
    camera.UpdateFront(45.0f, 30.0f);

    glm::vec3 front = camera.GetFront();
    float length = glm::length(front);

    // Normalized vector should have length of 1.0
    CHECK(FloatApproxEquals(length, 1.0f));
}

TEST_CASE("UpdateFront - Handles simultaneous horizontal and vertical movement")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;

    // Move diagonally
    camera.UpdateFront(15.0f, 20.0f);

    CHECK(FloatApproxEquals(camera.m_yaw, initialYaw + 15.0f));
    CHECK(FloatApproxEquals(camera.m_pitch, initialPitch + 20.0f));
}

TEST_CASE("UpdateFront - Handles negative offsets")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;

    // Move in negative directions
    camera.UpdateFront(-10.0f, -5.0f);

    CHECK(FloatApproxEquals(camera.m_yaw, initialYaw - 10.0f));
    CHECK(FloatApproxEquals(camera.m_pitch, initialPitch - 5.0f));
}

// ===================================================================
// UpdatePosition TESTS
// ===================================================================

TEST_CASE("UpdatePosition - Sets position correctly")
{
    Camera camera{};

    glm::vec3 newPosition(5.0f, 10.0f, -3.0f);
    camera.UpdatePosition(newPosition);

    glm::vec3 position = camera.GetPosition();
    CHECK(Vec3ApproxEquals(position, newPosition));
}

TEST_CASE("UpdatePosition - Can set position multiple times")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(1.0f, 2.0f, 3.0f));
    CHECK(Vec3ApproxEquals(camera.GetPosition(), glm::vec3(1.0f, 2.0f, 3.0f)));

    camera.UpdatePosition(glm::vec3(10.0f, 20.0f, 30.0f));
    CHECK(Vec3ApproxEquals(camera.GetPosition(), glm::vec3(10.0f, 20.0f, 30.0f)));
}

TEST_CASE("UpdatePosition - Handles zero position")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 0.0f));
    glm::vec3 position = camera.GetPosition();
    CHECK(Vec3ApproxEquals(position, glm::vec3(0.0f, 0.0f, 0.0f)));
}

TEST_CASE("UpdatePosition - Handles negative coordinates")
{
    Camera camera{};

    glm::vec3 negativePos(-5.0f, -10.0f, -15.0f);
    camera.UpdatePosition(negativePos);

    CHECK(Vec3ApproxEquals(camera.GetPosition(), negativePos));
}

// ===================================================================
// GetFront TESTS
// ===================================================================

TEST_CASE("GetFront - Returns normalized direction vector")
{
    Camera camera{};

    // Apply various rotations
    camera.UpdateFront(45.0f, 30.0f);

    glm::vec3 front = camera.GetFront();
    float length = glm::length(front);

    CHECK(FloatApproxEquals(length, 1.0f));
}

TEST_CASE("GetFront - Direction changes after UpdateFront")
{
    Camera camera{};

    glm::vec3 initialFront = camera.GetFront();

    // Rotate camera
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 newFront = camera.GetFront();

    // Front should have changed
    CHECK(!Vec3ApproxEquals(initialFront, newFront));
}

// ===================================================================
// GetUp TESTS
// ===================================================================

TEST_CASE("GetUp - Returns constant up vector")
{
    Camera camera{};

    glm::vec3 up1 = camera.GetUp();
    CHECK(Vec3ApproxEquals(up1, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Rotate camera
    camera.UpdateFront(45.0f, 30.0f);

    glm::vec3 up2 = camera.GetUp();
    // Up vector should remain constant
    CHECK(Vec3ApproxEquals(up2, glm::vec3(0.0f, 1.0f, 0.0f)));
    CHECK(Vec3ApproxEquals(up1, up2));
}

// ===================================================================
// GetRight TESTS
// ===================================================================

TEST_CASE("GetRight - Returns normalized right vector")
{
    Camera camera{};

    glm::vec3 right = camera.GetRight();
    float length = glm::length(right);

    CHECK(FloatApproxEquals(length, 1.0f));
}

TEST_CASE("GetRight - Right vector is perpendicular to front and up")
{
    Camera camera{};

    glm::vec3 front = camera.GetFront();
    glm::vec3 up = camera.GetUp();
    glm::vec3 right = camera.GetRight();

    // Right should be perpendicular to front (dot product = 0)
    float dotFront = glm::dot(right, front);
    CHECK(FloatApproxEquals(dotFront, 0.0f));

    // Right should be perpendicular to up (dot product = 0)
    float dotUp = glm::dot(right, up);
    CHECK(FloatApproxEquals(dotUp, 0.0f));
}

TEST_CASE("GetRight - Right vector changes when camera rotates")
{
    Camera camera{};

    glm::vec3 right1 = camera.GetRight();

    // Rotate camera
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 right2 = camera.GetRight();

    // Right vector should have changed
    CHECK(!Vec3ApproxEquals(right1, right2));
}

TEST_CASE("GetRight - Right vector follows right-hand rule")
{
    Camera camera{};

    glm::vec3 front = camera.GetFront();
    glm::vec3 up = camera.GetUp();
    glm::vec3 right = camera.GetRight();

    // Right should equal cross(front, up) normalized
    glm::vec3 expectedRight = glm::normalize(glm::cross(front, up));

    CHECK(Vec3ApproxEquals(right, expectedRight));
}

// ===================================================================
// GetCameraView TESTS
// ===================================================================

TEST_CASE("GetCameraView - Returns valid view matrix")
{
    Camera camera{};

    glm::mat4 view = camera.GetCameraView();

    // View matrix should be a valid transformation matrix
    // Check that determinant is non-zero (valid transformation)
    float determinant = glm::determinant(view);
    CHECK(determinant != 0.0f);

    // View matrix should have reasonable values (not NaN or Inf)
    bool hasValidValues = true;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float val = view[i][j];
            if (std::isnan(val) || std::isinf(val))
            {
                hasValidValues = false;
                break;
            }
        }
        if (!hasValidValues)
            break;
    }
    CHECK(hasValidValues == true);
}

TEST_CASE("GetCameraView - View matrix changes with position")
{
    Camera camera{};

    glm::mat4 view1 = camera.GetCameraView();

    camera.UpdatePosition(glm::vec3(5.0f, 0.0f, 0.0f));
    glm::mat4 view2 = camera.GetCameraView();

    // View matrices should be different
    CHECK(!Mat4ApproxEquals(view1, view2));
}

TEST_CASE("GetCameraView - View matrix changes with rotation")
{
    Camera camera{};

    glm::mat4 view1 = camera.GetCameraView();

    camera.UpdateFront(45.0f, 30.0f);
    glm::mat4 view2 = camera.GetCameraView();

    // View matrices should be different
    CHECK(!Mat4ApproxEquals(view1, view2));
}

TEST_CASE("GetCameraView - Uses correct look-at parameters")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 5.0f));
    camera.UpdateFront(0.0f, 0.0f); // Keep default direction

    glm::mat4 view = camera.GetCameraView();

    // Compare against expected glm::lookAt result
    glm::vec3 pos = camera.GetPosition();
    glm::vec3 front = camera.GetFront();
    glm::vec3 up = camera.GetUp();
    glm::mat4 expected = glm::lookAt(pos, pos + front, up);

    CHECK(Mat4ApproxEquals(view, expected));
}

// ===================================================================
// INTEGRATION TESTS
// ===================================================================

TEST_CASE("Integration - Camera movement and rotation work together")
{
    Camera camera{};

    // Set position
    camera.UpdatePosition(glm::vec3(10.0f, 5.0f, -3.0f));
    CHECK(Vec3ApproxEquals(camera.GetPosition(), glm::vec3(10.0f, 5.0f, -3.0f)));

    // Rotate camera
    camera.UpdateFront(45.0f, 30.0f);

    // Verify front direction changed
    glm::vec3 front = camera.GetFront();
    CHECK(FloatApproxEquals(glm::length(front), 1.0f)); // Should be normalized

    // Verify view matrix is valid
    glm::mat4 view = camera.GetCameraView();
    CHECK(glm::determinant(view) != 0.0f);
}

TEST_CASE("Integration - Multiple rotations accumulate correctly")
{
    Camera camera{};

    // Rotate multiple times
    camera.UpdateFront(10.0f, 5.0f);
    float yaw1 = camera.m_yaw;
    float pitch1 = camera.m_pitch;

    camera.UpdateFront(10.0f, 5.0f);
    float yaw2 = camera.m_yaw;
    float pitch2 = camera.m_pitch;

    // Yaw and pitch should accumulate
    CHECK(FloatApproxEquals(yaw2 - yaw1, 10.0f));
    CHECK(FloatApproxEquals(pitch2 - pitch1, 5.0f));
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE("Edge case - Very small offsets")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;

    camera.UpdateFront(0.001f, 0.001f);

    CHECK(FloatApproxEquals(camera.m_yaw, initialYaw + 0.001f));
    CHECK(FloatApproxEquals(camera.m_pitch, initialPitch + 0.001f));
}

TEST_CASE("Edge case - Very large offsets")
{
    Camera camera{};

    // Very large yaw offset (should work)
    camera.UpdateFront(1000.0f, 0.0f);

    // Yaw can exceed 360 degrees (it's not clamped)
    CHECK(camera.m_yaw > 900.0f);

    // Very large pitch offset (should be clamped)
    camera.UpdateFront(0.0f, 1000.0f);
    CHECK(FloatApproxEquals(camera.m_pitch, camera.m_maxRange));
}

TEST_CASE("Edge case - Zero offsets")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;
    glm::vec3 initialFront = camera.GetFront();

    camera.UpdateFront(0.0f, 0.0f);

    CHECK(FloatApproxEquals(camera.m_yaw, initialYaw));
    CHECK(FloatApproxEquals(camera.m_pitch, initialPitch));
    CHECK(Vec3ApproxEquals(camera.GetFront(), initialFront));
}

// ===================================================================
// LookAt TESTS
// ===================================================================

TEST_CASE("Camera - LookAt sets front toward target along positive X axis")
{
    Camera camera{};
    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 0.0f));

    camera.LookAt(glm::vec3(5.0f, 0.0f, 0.0f));

    CHECK(Vec3ApproxEquals(camera.GetFront(), glm::vec3(1.0f, 0.0f, 0.0f)));
    CHECK(FloatApproxEquals(camera.m_yaw, 0.0f));
    CHECK(FloatApproxEquals(camera.m_pitch, 0.0f));
}

TEST_CASE("Camera - LookAt sets front toward target along positive Z axis")
{
    Camera camera{};
    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 0.0f));

    camera.LookAt(glm::vec3(0.0f, 0.0f, 5.0f));

    CHECK(Vec3ApproxEquals(camera.GetFront(), glm::vec3(0.0f, 0.0f, 1.0f)));
    CHECK(FloatApproxEquals(camera.m_yaw, 90.0f));
    CHECK(FloatApproxEquals(camera.m_pitch, 0.0f));
}

TEST_CASE("Camera - LookAt sets pitch when target is elevated")
{
    Camera camera{};
    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 0.0f));

    // Target elevated along negative Z
    camera.LookAt(glm::vec3(0.0f, 1.0f, -1.0f));

    CHECK(FloatApproxEquals(camera.m_pitch, 45.0f));
    CHECK(FloatApproxEquals(camera.m_yaw, -90.0f));
    CHECK(FloatApproxEquals(glm::length(camera.GetFront()), 1.0f));
}
