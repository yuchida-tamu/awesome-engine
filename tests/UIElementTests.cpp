#include "doctest.h"
#include "ui/UIElement.h"
#include "ui/TextElement.h"

// A minimal concrete UIElement for testing the base class interface.
class StubUIElement : public UIElement {
public:
  void Render(Shader &shader) override {}
  void Update(float deltaTime) override {}
};

// ===================================================================
// POSITION / SIZE TESTS
// ===================================================================

TEST_CASE("UIElement - Default position is (0, 0)") {
  StubUIElement element;
  CHECK(element.GetPosition().x == doctest::Approx(0.0f));
  CHECK(element.GetPosition().y == doctest::Approx(0.0f));
}

TEST_CASE("UIElement - SetPosition updates position") {
  StubUIElement element;
  element.SetPosition({100.0f, 200.0f});
  CHECK(element.GetPosition().x == doctest::Approx(100.0f));
  CHECK(element.GetPosition().y == doctest::Approx(200.0f));
}

TEST_CASE("UIElement - Default size is (0, 0)") {
  StubUIElement element;
  CHECK(element.GetSize().x == doctest::Approx(0.0f));
  CHECK(element.GetSize().y == doctest::Approx(0.0f));
}

TEST_CASE("UIElement - SetSize updates size") {
  StubUIElement element;
  element.SetSize({320.0f, 240.0f});
  CHECK(element.GetSize().x == doctest::Approx(320.0f));
  CHECK(element.GetSize().y == doctest::Approx(240.0f));
}

// ===================================================================
// VISIBILITY TESTS
// ===================================================================

TEST_CASE("UIElement - Visible by default") {
  StubUIElement element;
  CHECK(element.IsVisible() == true);
}

TEST_CASE("UIElement - SetVisible toggles visibility") {
  StubUIElement element;
  element.SetVisible(false);
  CHECK(element.IsVisible() == false);
  element.SetVisible(true);
  CHECK(element.IsVisible() == true);
}

// ===================================================================
// POLYMORPHISM TESTS
// ===================================================================

TEST_CASE("UIElement - TextElement can be stored as UIElement pointer") {
  // Verifies that TextElement is a valid UIElement subclass
  // and can be managed polymorphically (e.g., in a vector<unique_ptr<UIElement>>).
  std::unique_ptr<UIElement> element = std::make_unique<TextElement>();
  CHECK(element->IsVisible() == true);
  element->SetPosition({50.0f, 75.0f});
  CHECK(element->GetPosition().x == doctest::Approx(50.0f));
}
