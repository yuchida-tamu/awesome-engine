#include "core/InputEvents.h"
#include "doctest.h"
#include "ui/ButtonElement.h"

// ===================================================================
// COLOR TESTS
// ===================================================================

TEST_CASE("ButtonElement - Default color is white") {
  ButtonElement button;
  CHECK(button.GetColor().r == doctest::Approx(1.0f));
  CHECK(button.GetColor().g == doctest::Approx(1.0f));
  CHECK(button.GetColor().b == doctest::Approx(1.0f));
}

TEST_CASE("ButtonElement - SetColor stores the color") {
  ButtonElement button;
  button.SetColor({0.2f, 0.6f, 1.0f});
  CHECK(button.GetColor().r == doctest::Approx(0.2f));
  CHECK(button.GetColor().g == doctest::Approx(0.6f));
  CHECK(button.GetColor().b == doctest::Approx(1.0f));
}

// ===================================================================
// CLICK TESTS
// ===================================================================

TEST_CASE("ButtonElement - OnClick triggers registered callback") {
  ButtonElement button;
  bool clicked = false;
  button.SetOnClick([&clicked](const MouseClickEvent &) { clicked = true; });

  MouseClickEvent event{100.0f, 200.0f, 0, KeyAction::Down};
  button.OnClick(event);

  CHECK(clicked == true);
}

TEST_CASE("ButtonElement - OnClick is safe with no callback") {
  ButtonElement button;
  MouseClickEvent event{100.0f, 200.0f, 0, KeyAction::Down};
  // Should not crash
  button.OnClick(event);
}

// ===================================================================
// INHERITED UIElement BEHAVIOR
// ===================================================================

TEST_CASE("ButtonElement - Inherits UIElement position/size/visibility") {
  ButtonElement button;
  button.SetPosition({100.0f, 200.0f});
  button.SetSize({300.0f, 50.0f});
  button.SetVisible(false);

  CHECK(button.GetPosition().x == doctest::Approx(100.0f));
  CHECK(button.GetPosition().y == doctest::Approx(200.0f));
  CHECK(button.GetSize().x == doctest::Approx(300.0f));
  CHECK(button.GetSize().y == doctest::Approx(50.0f));
  CHECK(button.IsVisible() == false);
}

TEST_CASE("ButtonElement - ContainPoint works with position and size") {
  ButtonElement button;
  button.SetPosition({100.0f, 200.0f});
  button.SetSize({200.0f, 50.0f});

  // Inside
  CHECK(button.ContainPoint({150.0f, 225.0f}) == true);
  // On edges
  CHECK(button.ContainPoint({100.0f, 200.0f}) == true);
  CHECK(button.ContainPoint({300.0f, 250.0f}) == true);
  // Outside
  CHECK(button.ContainPoint({99.0f, 225.0f}) == false);
  CHECK(button.ContainPoint({301.0f, 225.0f}) == false);
  CHECK(button.ContainPoint({150.0f, 199.0f}) == false);
  CHECK(button.ContainPoint({150.0f, 251.0f}) == false);
}
