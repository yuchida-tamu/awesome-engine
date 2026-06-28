#include "core/EntryPoint.h"

#include "EditorLayer.h"
#include "core/Application.h"
#include <memory>
class EditorApp : public Application {
public:
  EditorApp() { PushLayer(std::make_unique<EditorLayer>()); }
};

Application *CreateApplication() { return new EditorApp(); }
