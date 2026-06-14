#include "TestHelpers.h"
#include "doctest.h"

#include "debug/FpsCounter.h"

// NOTE: these tests use exact binary fractions (1/4, 1/8, 1/16) for the window
// and dt so the accumulated time hits the window boundary precisely. Using
// values like 0.01f would accumulate rounding error and land just shy of the
// boundary, making the refresh fire a frame late.

// ===================================================================
// WINDOWED AVERAGING
// ===================================================================

TEST_CASE("FpsCounter - reports 0 before the first window completes") {
  FpsCounter c(0.25f);
  CHECK(c.Fps() == doctest::Approx(0.0f));
  c.Tick(0.125f); // 0.125 < 0.25 window
  CHECK(c.Fps() == doctest::Approx(0.0f));
}

TEST_CASE("FpsCounter - averages frames over the window") {
  FpsCounter c(0.25f);
  // 4 frames * 0.0625s = 0.25s = one full window -> 16 fps
  for (int i = 0; i < 4; ++i)
    c.Tick(0.0625f);
  CHECK(c.Fps() == doctest::Approx(16.0f));
}

TEST_CASE("FpsCounter - holds its value between refreshes") {
  FpsCounter c(0.25f);
  for (int i = 0; i < 4; ++i)
    c.Tick(0.0625f); // refresh -> 16 fps
  CHECK(c.Fps() == doctest::Approx(16.0f));

  // Two sub-window ticks (0.125s total) must not change the value yet.
  c.Tick(0.0625f);
  c.Tick(0.0625f);
  CHECK(c.Fps() == doctest::Approx(16.0f));
}

TEST_CASE("FpsCounter - recomputes on the next window") {
  FpsCounter c(0.25f);
  for (int i = 0; i < 4; ++i)
    c.Tick(0.0625f); // window 1: 4 frames / 0.25s -> 16 fps
  CHECK(c.Fps() == doctest::Approx(16.0f));

  c.Tick(0.125f);
  c.Tick(0.125f); // window 2: 2 frames / 0.25s -> 8 fps
  CHECK(c.Fps() == doctest::Approx(8.0f));
}
