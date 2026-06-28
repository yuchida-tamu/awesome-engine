#include "core/EntryPoint.h"

#include "core/Application.h"
#include "GameLayer.h"
#include <memory>
class GameApp : public Application {
public:
  GameApp() { PushLayer(std::make_unique<GameLayer>()); }
};

Application *CreateApplication() { return new GameApp(); }
