#include "doctest.h"
#include "ui/TextElement.h"

// ===================================================================
// TEXT CONTENT TESTS
// ===================================================================

TEST_CASE("TextElement - SetText stores the text string") {
  TextElement text;
  text.SetText("Hello, World!");
  CHECK(text.GetText() == "Hello, World!");
}

TEST_CASE("TextElement - Default text is empty") {
  TextElement text;
  CHECK(text.GetText().empty());
}

// ===================================================================
// COLOR TESTS
// ===================================================================

TEST_CASE("TextElement - SetColor stores the color") {
  TextElement text;
  text.SetColor({1.0f, 0.0f, 0.5f});
  CHECK(text.GetColor().r == doctest::Approx(1.0f));
  CHECK(text.GetColor().g == doctest::Approx(0.0f));
  CHECK(text.GetColor().b == doctest::Approx(0.5f));
}

TEST_CASE("TextElement - Default color is white") {
  TextElement text;
  CHECK(text.GetColor().r == doctest::Approx(1.0f));
  CHECK(text.GetColor().g == doctest::Approx(1.0f));
  CHECK(text.GetColor().b == doctest::Approx(1.0f));
}

// ===================================================================
// SCALE TESTS
// ===================================================================

TEST_CASE("TextElement - SetScale stores the scale") {
  TextElement text;
  text.SetScale(2.5f);
  CHECK(text.GetScale() == doctest::Approx(2.5f));
}

TEST_CASE("TextElement - Default scale is 1.0") {
  TextElement text;
  CHECK(text.GetScale() == doctest::Approx(1.0f));
}

// ===================================================================
// MEASURE WIDTH TESTS
// ===================================================================

TEST_CASE("TextElement - MeasureWidth returns 0 for empty text") {
  TextElement text;
  CHECK(text.MeasureWidth() == doctest::Approx(0.0f));
}

// ===================================================================
// INHERITED UIElement BEHAVIOR
// ===================================================================

TEST_CASE("TextElement - Inherits UIElement position/visibility") {
  TextElement text;
  text.SetPosition({10.0f, 20.0f});
  text.SetVisible(false);

  CHECK(text.GetPosition().x == doctest::Approx(10.0f));
  CHECK(text.GetPosition().y == doctest::Approx(20.0f));
  CHECK(text.IsVisible() == false);
}
