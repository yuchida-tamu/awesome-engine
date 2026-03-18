// This #define tells doctest to provide a main() function for our test executable.
// You should only have this in ONE of your test files.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <cstring> // For memset
#include <stdexcept> // For std::logic_error (used by ENGINE_ASSERT in test builds)

// We need to include the class we want to test.
#include "core/Config.h"
#include "core/Input.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"

static EventBus s_TestEventBus;

// Helper function to reset Input state between tests
// This ensures test isolation
void ResetInputState()
{
    memset(Input::s_Keys, 0, Config::MAX_KEYS * sizeof(bool));
    memset(Input::s_KeysLastFrame, 0, Config::MAX_KEYS * sizeof(bool));
    memset(Input::s_KeysRaw, 0, Config::MAX_KEYS * sizeof(bool));
    memset(Input::s_MouseButtons, 0, Config::MAX_MOUSE_BUTTONS * sizeof(bool));
    memset(Input::s_MouseButtonsLastFrame, 0, Config::MAX_MOUSE_BUTTONS * sizeof(bool));
    memset(Input::s_MouseButtonsRaw, 0, Config::MAX_MOUSE_BUTTONS * sizeof(bool));

    Input::s_FirstMouse = true;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;
    Input::s_XOffset = 0.0f;
    Input::s_YOffset = 0.0f;
    Input::s_EventBus = &s_TestEventBus;
}

// Helper to set up mouse state for movement tests
void SetupMouseState()
{
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;
}

// ===================================================================
// KEYBOARD INPUT TESTS
// ===================================================================

TEST_CASE("Input - IsKeyDown returns true only on first frame key is pressed")
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

TEST_CASE("Input - IsKeyHeld returns true while key is held down")
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

TEST_CASE("Input - IsKeyHeld returns false when key is not pressed")
{
    ResetInputState();

    // No key pressed
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_A) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_B) == false);
    CHECK(Input::IsKeyHeld(GLFW_KEY_ESCAPE) == false);
}

TEST_CASE("Input - Multiple keys can be held simultaneously")
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

TEST_CASE("Input - Rapid key press and release")
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

TEST_CASE("Input - GLFW_REPEAT action is ignored")
{
    ResetInputState();

    // GLFW_REPEAT should not change key state
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_REPEAT, 0);
    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == false);
    CHECK(Input::IsKeyDown(GLFW_KEY_W) == false);
}

// ===================================================================
// MOUSE INPUT TESTS
// ===================================================================

TEST_CASE("Input - GetMouseOffset returns zero when mouse hasn't moved")
{
    ResetInputState();

    // No mouse movement
    Input::Update();
    glm::vec2 offset = Input::GetMouseOffset();

    CHECK(offset.x == 0.0f);
    CHECK(offset.y == 0.0f);
}

TEST_CASE("Input - GetMouseOffset calculates offset correctly after mouse movement")
{
    ResetInputState();
    SetupMouseState();

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

TEST_CASE("Input - GetMouseOffset resets to zero when mouse stops moving")
{
    ResetInputState();
    SetupMouseState();

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

TEST_CASE("Input - MouseCallBack handles first mouse event correctly")
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

TEST_CASE("Input - MouseCallBack updates current position correctly")
{
    ResetInputState();
    SetupMouseState();

    // Move mouse
    Input::MouseCallBack(nullptr, 450.0, 350.0);

    CHECK(Input::s_CurrentX == 450.0);
    CHECK(Input::s_CurrentY == 350.0);
    CHECK(Input::s_LastX == 400.0); // Should not change until Update()
}

TEST_CASE("Input - Mouse offset calculation with sensitivity")
{
    ResetInputState();
    SetupMouseState();

    // Move mouse diagonally
    Input::MouseCallBack(nullptr, 500.0, 400.0);
    Input::Update();

    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 100.0f);
    CHECK(offset.y == -100.0f); // Y is inverted
}

TEST_CASE("Input - MouseMoveEvent includes sensitivity scaling")
{
    ResetInputState();
    SetupMouseState();

    float receivedX = 0.0f, receivedY = 0.0f;
    s_TestEventBus.Subscribe<MouseMoveEvent>(
        std::function<void(const MouseMoveEvent &)>([&](const MouseMoveEvent &e) {
            receivedX = e.xOffset;
            receivedY = e.yOffset;
        }));

    Input::MouseCallBack(nullptr, 500.0, 400.0);
    Input::Update();

    // Raw offset is 100, -100. Sensitivity is 0.1f.
    CHECK(receivedX == doctest::Approx(100.0f * Input::s_Sensitivity));
    CHECK(receivedY == doctest::Approx(-100.0f * Input::s_Sensitivity));
}

TEST_CASE("Input - Update correctly updates mouse offset and resets for next frame")
{
    ResetInputState();
    SetupMouseState();

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

TEST_CASE("Input - Update handles keyboard and mouse state together")
{
    ResetInputState();
    SetupMouseState();

    // Press key and move mouse simultaneously
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_PRESS, 0);
    Input::MouseCallBack(nullptr, 450.0, 300.0);

    Input::Update();

    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == true);
    glm::vec2 offset = Input::GetMouseOffset();
    CHECK(offset.x == 50.0f);
}

// ===================================================================
// MOUSE BUTTON TESTS
// ===================================================================

TEST_CASE("Input - IsMouseButtonDown returns true only on first frame button is pressed")
{
    ResetInputState();

    Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);

    // Frame 1: First frame after press
    Input::Update();
    CHECK(Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) == true);
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) == true);

    // Frame 2: Still held, but not "just pressed"
    Input::Update();
    CHECK(Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) == false);
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) == true);

    // Frame 3: Release
    Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0);
    Input::Update();
    CHECK(Input::IsMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT) == false);
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_LEFT) == false);
}

TEST_CASE("Input - IsMouseButtonHeld returns true while button is held")
{
    ResetInputState();

    Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0);
    Input::Update();
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) == true);

    Input::Update();
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) == true);

    Input::Update();
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) == true);

    Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, 0);
    Input::Update();
    CHECK(Input::IsMouseButtonHeld(GLFW_MOUSE_BUTTON_RIGHT) == false);
}

TEST_CASE("Input - MouseClickEvent published with correct action and position")
{
    ResetInputState();
    SetupMouseState();

    // Move cursor to a known position
    Input::MouseCallBack(nullptr, 150.0, 250.0);

    int receivedButton = -1;
    KeyAction receivedAction = KeyAction::Held;
    float receivedX = 0.0f, receivedY = 0.0f;

    s_TestEventBus.Subscribe<MouseClickEvent>(
        std::function<void(const MouseClickEvent &)>([&](const MouseClickEvent &e) {
            if (e.key == KeyAction::Down) {
                receivedButton = e.button;
                receivedAction = e.key;
                receivedX = e.xPos;
                receivedY = e.yPos;
            }
        }));

    Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
    Input::Update();

    CHECK(receivedButton == GLFW_MOUSE_BUTTON_LEFT);
    CHECK(receivedAction == KeyAction::Down);
    CHECK(receivedX == doctest::Approx(150.0f));
    CHECK(receivedY == doctest::Approx(250.0f));
}

TEST_CASE("Input - MouseButtonCallBack ignores invalid button codes")
{
    ResetInputState();

    // Should not crash or cause UB
    Input::MouseButtonCallBack(nullptr, -1, GLFW_PRESS, 0);
    Input::MouseButtonCallBack(nullptr, Config::MAX_MOUSE_BUTTONS, GLFW_PRESS, 0);
    Input::Update();

    // State should remain clean
    CHECK(Input::IsMouseButtonHeld(0) == false);
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE("Input - Invalid key codes return false without UB")
{
    ResetInputState();

    Input::Update();

    // Very large key code (out of bounds)
    CHECK(Input::IsKeyHeld(9999) == false);
    CHECK(Input::IsKeyDown(9999) == false);

    // Negative key code (GLFW_KEY_UNKNOWN = -1)
    CHECK(Input::IsKeyHeld(-1) == false);
    CHECK(Input::IsKeyDown(-1) == false);
}

TEST_CASE("Input - KeyCallBack ignores invalid key codes")
{
    ResetInputState();

    // Should not crash or cause UB
    Input::KeyCallBack(nullptr, -1, 0, GLFW_PRESS, 0);
    Input::KeyCallBack(nullptr, Config::MAX_KEYS, 0, GLFW_PRESS, 0);
    Input::Update();

    // State should remain clean
    CHECK(Input::IsKeyHeld(0) == false);
}

TEST_CASE("Input - Update aborts with message when EventBus is not initialized")
{
    ResetInputState();
    Input::s_EventBus = nullptr;

    CHECK_THROWS_AS(Input::Update(), std::logic_error);
}

TEST_CASE("Input - Mouse movement at screen boundaries")
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
