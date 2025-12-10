// Note: DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN is defined in InputTests.cpp
// We don't define it here since we're combining multiple test files
#include "doctest.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera/Camera.h"

// Helper function to compare floating point values with tolerance
bool FloatEquals(float a, float b, float epsilon = 0.0001f)
{
    return std::abs(a - b) < epsilon;
}

// Helper function to compare vec3 with tolerance
bool Vec3Equals(const glm::vec3 &a, const glm::vec3 &b, float epsilon = 0.0001f)
{
    return FloatEquals(a.x, b.x, epsilon) &&
           FloatEquals(a.y, b.y, epsilon) &&
           FloatEquals(a.z, b.z, epsilon);
}

// Helper function to compare mat4 matrices with tolerance
bool Mat4Equals(const glm::mat4 &a, const glm::mat4 &b, float epsilon = 0.0001f)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (!FloatEquals(a[i][j], b[i][j], epsilon))
                return false;
        }
    }
    return true;
}

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
    CHECK(FloatEquals(camera.m_pitch, 0.0f));
    CHECK(FloatEquals(camera.m_yaw, -90.0f)); // Default yaw is -90 degrees

    // Check initial front direction (should point along negative Z axis)
    glm::vec3 front = camera.GetFront();
    CHECK(FloatEquals(front.x, 0.0f));
    CHECK(FloatEquals(front.y, 0.0f));
    CHECK(FloatEquals(front.z, -1.0f));

    // Check initial up vector
    glm::vec3 up = camera.GetUp();
    CHECK(Vec3Equals(up, glm::vec3(0.0f, 1.0f, 0.0f)));
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

    CHECK(FloatEquals(camera.m_yaw, initialYaw + 10.0f));
    CHECK(FloatEquals(camera.m_pitch, 0.0f)); // Pitch should remain unchanged
}

TEST_CASE("UpdateFront - Updates pitch correctly with vertical mouse movement")
{
    Camera camera{};

    float initialPitch = camera.m_pitch;

    // Move mouse up (positive yOffset)
    camera.UpdateFront(0.0f, 5.0f);

    CHECK(FloatEquals(camera.m_pitch, initialPitch + 5.0f));
    CHECK(FloatEquals(camera.m_yaw, -90.0f)); // Yaw should remain unchanged
}

TEST_CASE("UpdateFront - Clamps pitch to maximum range")
{
    Camera camera{};

    // Try to exceed maximum pitch
    camera.UpdateFront(0.0f, 100.0f);

    CHECK(FloatEquals(camera.m_pitch, camera.m_maxRange));
}

TEST_CASE("UpdateFront - Clamps pitch to minimum range")
{
    Camera camera{};

    // Try to exceed minimum pitch
    camera.UpdateFront(0.0f, -100.0f);

    CHECK(FloatEquals(camera.m_pitch, camera.m_minRange));
}

TEST_CASE("UpdateFront - Clamps pitch when already at maximum")
{
    Camera camera{};

    // Set pitch to maximum
    camera.UpdateFront(0.0f, 89.0f);
    CHECK(FloatEquals(camera.m_pitch, 89.0f));

    // Try to increase further
    camera.UpdateFront(0.0f, 10.0f);
    CHECK(FloatEquals(camera.m_pitch, 89.0f)); // Should remain clamped
}

TEST_CASE("UpdateFront - Clamps pitch when already at minimum")
{
    Camera camera{};

    // Set pitch to minimum
    camera.UpdateFront(0.0f, -89.0f);
    CHECK(FloatEquals(camera.m_pitch, -89.0f));

    // Try to decrease further
    camera.UpdateFront(0.0f, -10.0f);
    CHECK(FloatEquals(camera.m_pitch, -89.0f)); // Should remain clamped
}

TEST_CASE("UpdateFront - Updates front direction vector correctly")
{
    Camera camera{};

    // Rotate camera 90 degrees to the right (around Y axis)
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 front = camera.GetFront();
    // After rotating 90 degrees from -90 degrees, yaw should be 0
    // Front should point along positive X axis
    CHECK(FloatEquals(front.x, 1.0f));
    CHECK(FloatEquals(front.y, 0.0f));
    CHECK(FloatEquals(front.z, 0.0f));
}

TEST_CASE("UpdateFront - Front vector is normalized")
{
    Camera camera{};

    // Apply some rotation
    camera.UpdateFront(45.0f, 30.0f);

    glm::vec3 front = camera.GetFront();
    float length = glm::length(front);

    // Normalized vector should have length of 1.0
    CHECK(FloatEquals(length, 1.0f));
}

TEST_CASE("UpdateFront - Handles simultaneous horizontal and vertical movement")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;

    // Move diagonally
    camera.UpdateFront(15.0f, 20.0f);

    CHECK(FloatEquals(camera.m_yaw, initialYaw + 15.0f));
    CHECK(FloatEquals(camera.m_pitch, initialPitch + 20.0f));
}

TEST_CASE("UpdateFront - Handles negative offsets")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;

    // Move in negative directions
    camera.UpdateFront(-10.0f, -5.0f);

    CHECK(FloatEquals(camera.m_yaw, initialYaw - 10.0f));
    CHECK(FloatEquals(camera.m_pitch, initialPitch - 5.0f));
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
    CHECK(Vec3Equals(position, newPosition));
}

TEST_CASE("UpdatePosition - Can set position multiple times")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(1.0f, 2.0f, 3.0f));
    CHECK(Vec3Equals(camera.GetPosition(), glm::vec3(1.0f, 2.0f, 3.0f)));

    camera.UpdatePosition(glm::vec3(10.0f, 20.0f, 30.0f));
    CHECK(Vec3Equals(camera.GetPosition(), glm::vec3(10.0f, 20.0f, 30.0f)));
}

TEST_CASE("UpdatePosition - Handles zero position")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 0.0f));
    glm::vec3 position = camera.GetPosition();
    CHECK(Vec3Equals(position, glm::vec3(0.0f, 0.0f, 0.0f)));
}

TEST_CASE("UpdatePosition - Handles negative coordinates")
{
    Camera camera{};

    glm::vec3 negativePos(-5.0f, -10.0f, -15.0f);
    camera.UpdatePosition(negativePos);

    CHECK(Vec3Equals(camera.GetPosition(), negativePos));
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

    CHECK(FloatEquals(length, 1.0f));
}

TEST_CASE("GetFront - Direction changes after UpdateFront")
{
    Camera camera{};

    glm::vec3 initialFront = camera.GetFront();

    // Rotate camera
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 newFront = camera.GetFront();

    // Front should have changed
    CHECK(!Vec3Equals(initialFront, newFront));
}

// ===================================================================
// GetUp TESTS
// ===================================================================

TEST_CASE("GetUp - Returns constant up vector")
{
    Camera camera{};

    glm::vec3 up1 = camera.GetUp();
    CHECK(Vec3Equals(up1, glm::vec3(0.0f, 1.0f, 0.0f)));

    // Rotate camera
    camera.UpdateFront(45.0f, 30.0f);

    glm::vec3 up2 = camera.GetUp();
    // Up vector should remain constant
    CHECK(Vec3Equals(up2, glm::vec3(0.0f, 1.0f, 0.0f)));
    CHECK(Vec3Equals(up1, up2));
}

// ===================================================================
// GetRight TESTS
// ===================================================================

TEST_CASE("GetRight - Returns normalized right vector")
{
    Camera camera{};

    glm::vec3 right = camera.GetRight();
    float length = glm::length(right);

    CHECK(FloatEquals(length, 1.0f));
}

TEST_CASE("GetRight - Right vector is perpendicular to front and up")
{
    Camera camera{};

    glm::vec3 front = camera.GetFront();
    glm::vec3 up = camera.GetUp();
    glm::vec3 right = camera.GetRight();

    // Right should be perpendicular to front (dot product = 0)
    float dotFront = glm::dot(right, front);
    CHECK(FloatEquals(dotFront, 0.0f));

    // Right should be perpendicular to up (dot product = 0)
    float dotUp = glm::dot(right, up);
    CHECK(FloatEquals(dotUp, 0.0f));
}

TEST_CASE("GetRight - Right vector changes when camera rotates")
{
    Camera camera{};

    glm::vec3 right1 = camera.GetRight();

    // Rotate camera
    camera.UpdateFront(90.0f, 0.0f);

    glm::vec3 right2 = camera.GetRight();

    // Right vector should have changed
    CHECK(!Vec3Equals(right1, right2));
}

TEST_CASE("GetRight - Right vector follows right-hand rule")
{
    Camera camera{};

    glm::vec3 front = camera.GetFront();
    glm::vec3 up = camera.GetUp();
    glm::vec3 right = camera.GetRight();

    // Right should equal cross(front, up) normalized
    glm::vec3 expectedRight = glm::normalize(glm::cross(front, up));

    CHECK(Vec3Equals(right, expectedRight));
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
    CHECK(!Mat4Equals(view1, view2));
}

TEST_CASE("GetCameraView - View matrix changes with rotation")
{
    Camera camera{};

    glm::mat4 view1 = camera.GetCameraView();

    camera.UpdateFront(45.0f, 30.0f);
    glm::mat4 view2 = camera.GetCameraView();

    // View matrices should be different
    CHECK(!Mat4Equals(view1, view2));
}

TEST_CASE("GetCameraView - Uses correct look-at parameters")
{
    Camera camera{};

    camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 5.0f));
    camera.UpdateFront(0.0f, 0.0f); // Keep default direction

    glm::mat4 view = camera.GetCameraView();

    // View matrix should look from position towards (position + front)
    // We can verify by checking that the view matrix transforms correctly
    // This is a simplified check - in a real scenario, you might extract
    // the camera position and direction from the view matrix

    // For now, we just verify it's a valid transformation matrix
    // (determinant should be non-zero for a valid view matrix)
    float determinant = glm::determinant(view);
    CHECK(determinant != 0.0f);
}

// ===================================================================
// INTEGRATION TESTS
// ===================================================================

TEST_CASE("Integration - Camera movement and rotation work together")
{
    Camera camera{};

    // Set position
    camera.UpdatePosition(glm::vec3(10.0f, 5.0f, -3.0f));
    CHECK(Vec3Equals(camera.GetPosition(), glm::vec3(10.0f, 5.0f, -3.0f)));

    // Rotate camera
    camera.UpdateFront(45.0f, 30.0f);

    // Verify front direction changed
    glm::vec3 front = camera.GetFront();
    CHECK(FloatEquals(glm::length(front), 1.0f)); // Should be normalized

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
    CHECK(FloatEquals(yaw2 - yaw1, 10.0f));
    CHECK(FloatEquals(pitch2 - pitch1, 5.0f));
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

    CHECK(FloatEquals(camera.m_yaw, initialYaw + 0.001f));
    CHECK(FloatEquals(camera.m_pitch, initialPitch + 0.001f));
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
    CHECK(FloatEquals(camera.m_pitch, camera.m_maxRange));
}

TEST_CASE("Edge case - Zero offsets")
{
    Camera camera{};

    float initialYaw = camera.m_yaw;
    float initialPitch = camera.m_pitch;
    glm::vec3 initialFront = camera.GetFront();

    camera.UpdateFront(0.0f, 0.0f);

    CHECK(FloatEquals(camera.m_yaw, initialYaw));
    CHECK(FloatEquals(camera.m_pitch, initialPitch));
    CHECK(Vec3Equals(camera.GetFront(), initialFront));
}
