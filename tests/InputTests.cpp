// This #define tells doctest to provide a main() function for our test executable.
// You should only have this in ONE of your test files.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <cstring> // For memset

// We need to include the class we want to test.
#include "core/Input.h"

// Helper function to reset Input state between tests
// This ensures test isolation
void ResetInputState()
{
    // Use explicit size (1024) since sizeof might not work on incomplete array types
    constexpr size_t KEY_ARRAY_SIZE = 1024;
    memset(Input::s_Keys, 0, KEY_ARRAY_SIZE * sizeof(bool));
    memset(Input::s_KeysLastFrame, 0, KEY_ARRAY_SIZE * sizeof(bool));
    memset(Input::s_KeysRaw, 0, KEY_ARRAY_SIZE * sizeof(bool));

    Input::s_FirstMouse = true;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;
    Input::s_XOffset = 0.0f;
    Input::s_YOffset = 0.0f;
}

// ===================================================================
// KEYBOARD INPUT TESTS
// ===================================================================

TEST_CASE("IsKeyDown - Returns true only on first frame key is pressed")
{
    ResetInputState();

    // Simulate key press
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_PRESS, 0);

    // Frame 1: First frame after press
    Input::Update();
    CHECK(Input::IsKeyDown(GLFW_KEY_W) == true);
    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);

    // Frame 2: Key still held, but not "just pressed"
    Input::Update();
    CHECK(Input::IsKeyDown(GLFW_KEY_W) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);

    // Frame 3: Release the key
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_RELEASE, 0);
    Input::Update();
    CHECK(Input::IsKeyDown(GLFW_KEY_W) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == false);
}

TEST_CASE("IsKeyHeld - Returns true while key is held down")
{
    ResetInputState();

    // Press key
    Input::KeyCallBack(nullptr, GLFW_KEY_SPACE, 0, GLFW_PRESS, 0);
    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_SPACE) == true);

    // Hold for multiple frames
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_SPACE) == true);

    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_SPACE) == true);

    // Release
    Input::KeyCallBack(nullptr, GLFW_KEY_SPACE, 0, GLFW_RELEASE, 0);
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_SPACE) == false);
}

TEST_CASE("IsKeyHeld - Returns false when key is not pressed")
{
    ResetInputState();

    // No key pressed
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_A) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_B) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_ESCAPE) == false);
}

TEST_CASE("Multiple keys can be held simultaneously")
{
    ResetInputState();

    // Press multiple keys
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_PRESS, 0);
    Input::KeyCallBack(nullptr, GLFW_KEY_A, 0, GLFW_PRESS, 0);
    Input::KeyCallBack(nullptr, GLFW_KEY_S, 0, GLFW_PRESS, 0);

    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);
    CHECK(Input::IsKeyHeld(GLFW_KEY_A) == true);
    CHECK(Input::IsKeyHeld(GLFW_KEY_S) == true);

    // Release one key
    Input::KeyCallBack(nullptr, GLFW_KEY_A, 0, GLFW_RELEASE, 0);
    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);
    CHECK(Input::IsKeyHeld(GLFW_KEY_A) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_S) == true);
}

TEST_CASE("Rapid key press and release")
{
    ResetInputState();

    // Rapid press/release cycle
    Input::KeyCallBack(nullptr, GLFW_KEY_X, 0, GLFW_PRESS, 0);
    Input::Update();
    CHECK(Input::IsKeyDown(GLFW_KEY_X) == true);

    Input::KeyCallBack(nullptr, GLFW_KEY_X, 0, GLFW_RELEASE, 0);
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_X) == false);

    // Press again immediately
    Input::KeyCallBack(nullptr, GLFW_KEY_X, 0, GLFW_PRESS, 0);
    Input::Update();
    CHECK(Input::IsKeyDown(GLFW_KEY_X) == true);
    CHECK(Input::IsKeyHeld(GLFW_KEY_X) == true);
}

// ===================================================================
// MOUSE INPUT TESTS
// ===================================================================

TEST_CASE("GetMouseOffset - Returns zero when mouse hasn't moved")
{
    ResetInputState();

    // No mouse movement
    Input::Update();
    glm::vec2 offset = Input::GetMouseOffset();

    CHECK(offset.x == 0.0f);
    CHECK(offset.y == 0.0f);
}

TEST_CASE("GetMouseOffset - Calculates offset correctly after mouse movement")
{
    ResetInputState();

    // Set up initial state (simulate first mouse event already happened)
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;

    // Simulate mouse movement to the right
    Input::MouseCallBack(nullptr, 450.0, 300.0); // Moved 50 pixels right
    Input::Update();

    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 50.0f);
    CHECK(offset.y == 0.0f);

    // Move down
    Input::MouseCallBack(nullptr, 450.0, 350.0); // Moved 50 pixels down
    Input::Update();

    offset = Input::GetMouseOffset();
    CHECK(offset.x == 0.0f);
    CHECK(offset.y == -50.0f); // Y is inverted
}

TEST_CASE("GetMouseOffset - Resets to zero when mouse stops moving")
{
    ResetInputState();

    // Set up initial state (simulate first mouse event already happened)
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;

    // Move mouse
    Input::MouseCallBack(nullptr, 500.0, 400.0);
    Input::Update();

    glm::vec2 offset = Input::GetMouseOffset();
    // Check that some movement occurred (at least one component is non-zero)
    bool movementOccurred = (offset.x != 0.0f || offset.y != 0.0f);
    CHECK(movementOccurred == true);

    // Mouse stops (no new callback)
    Input::Update();

    offset = Input::GetMouseOffset();
    CHECK(offset.x == 0.0f);
    CHECK(offset.y == 0.0f);
}

TEST_CASE("MouseCallBack - Handles first mouse event correctly")
{
    ResetInputState();

    // First mouse event should snap last position to current
    Input::MouseCallBack(nullptr, 600.0, 500.0);

    CHECK(Input::s_FirstMouse == false);
    CHECK(Input::s_CurrentX == 600.0);
    CHECK(Input::s_CurrentY == 500.0);
    CHECK(Input::s_LastX == 600.0); // Should be snapped to current
    CHECK(Input::s_LastY == 500.0);

    // Update should result in zero offset for first frame
    Input::Update();
    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 0.0f);
    CHECK(offset.y == 0.0f);
}

TEST_CASE("MouseCallBack - Updates current position correctly")
{
    ResetInputState();

    // Set initial state
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;

    // Move mouse
    Input::MouseCallBack(nullptr, 450.0, 350.0);

    CHECK(Input::s_CurrentX == 450.0);
    CHECK(Input::s_CurrentY == 350.0);
    CHECK(Input::s_LastX == 400.0); // Should not change until Update()
}

TEST_CASE("Mouse offset calculation with sensitivity")
{
    ResetInputState();

    // Note: Sensitivity is applied in MouseCallBack, but let's test the offset calculation
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;

    // Move mouse diagonally
    Input::MouseCallBack(nullptr, 500.0, 400.0);
    Input::Update();

    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 100.0f);
    CHECK(offset.y == -100.0f); // Y is inverted
}

TEST_CASE("Update - Correctly updates mouse offset and resets for next frame")
{
    ResetInputState();

    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;

    // Frame 1: Move mouse
    Input::MouseCallBack(nullptr, 450.0, 350.0);
    Input::Update();

    glm::vec2 offset1 = Input::GetMouseOffset();
    CHECK(offset1.x == 50.0f);
    CHECK(offset1.y == -50.0f);

    // Frame 2: Mouse doesn't move (no callback)
    Input::Update();

    glm::vec2 offset2 = Input::GetMouseOffset();
    CHECK(offset2.x == 0.0f);
    CHECK(offset2.y == 0.0f);

    // Verify last position was updated
    CHECK(Input::s_LastX == 450.0);
    CHECK(Input::s_LastY == 350.0);
}

TEST_CASE("Update - Handles keyboard and mouse state together")
{
    ResetInputState();

    // Set up initial mouse state (simulate first mouse event already happened)
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;

    // Press key and move mouse simultaneously
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_PRESS, 0);
    Input::MouseCallBack(nullptr, 450.0, 300.0);

    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);
    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 50.0f);
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE("Edge case - Invalid key codes")
{
    ResetInputState();

    Input::Update();

    // Very large key code
    CHECK(Input::IsKeyHeld(9999) == false);
    CHECK(Input::IsKeyDown(9999) == false);

    // Negative key code
    CHECK(Input::IsKeyHeld(-1) == false);
    CHECK(Input::IsKeyDown(-1) == false);
}

TEST_CASE("Edge case - Mouse movement at screen boundaries")
{
    ResetInputState();

    Input::s_FirstMouse = false;
    Input::s_LastX = 0.0;
    Input::s_LastY = 0.0;

    // Move to top-left corner
    Input::MouseCallBack(nullptr, 0.0, 0.0);
    Input::Update();

    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 0.0f);
    CHECK(offset.y == 0.0f);

    // Move to bottom-right corner
    Input::MouseCallBack(nullptr, 800.0, 600.0);
    Input::Update();

    offset = Input::GetMouseOffset();
    CHECK(offset.x == 800.0f);
    CHECK(offset.y == -600.0f); // Y inverted
}
