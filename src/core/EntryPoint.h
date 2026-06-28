#pragma once

#include "core/Application.h"

extern Application *CreateApplication();

int main() {
  auto *app = CreateApplication();
  app->Run();
  delete app;
}
