#include "doctest.h"
#include "TestHelpers.h"

#include "core/Timestep.h"

// Timestep is a thin wrapper around a frame delta in seconds: the engine
// computes it once per frame and hands it to Layer::OnUpdate. It converts
// implicitly to float (seconds) for convenience.

TEST_CASE("Timestep - default constructs to zero") {
  Timestep ts;
  CHECK(ts.Seconds() == doctest::Approx(0.0f));
  CHECK(static_cast<float>(ts) == doctest::Approx(0.0f));
}

TEST_CASE("Timestep - reports seconds and milliseconds") {
  Timestep ts(0.016f);
  CHECK(ts.Seconds() == doctest::Approx(0.016f));
  CHECK(ts.Milliseconds() == doctest::Approx(16.0f));
}

TEST_CASE("Timestep - converts implicitly to seconds as a float") {
  Timestep ts(0.5f);
  float dt = ts; // implicit conversion
  CHECK(dt == doctest::Approx(0.5f));
}
