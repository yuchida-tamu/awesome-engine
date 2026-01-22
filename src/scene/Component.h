#pragma once

class GameObject;
class RenderContext;
class Shader;

class Component {
public:
  virtual ~Component() = default;
  // These are not pure virtual function so that they are not requireed to be
  // defined. They are simply empty functions
  virtual void Update(float deltaTime) {}
  virtual void Draw(RenderContext &renderContext, const float *modelPtr) {}
  virtual bool IsRenderable() const { return false; }

  void SetOwner(GameObject *owner) { m_owner = owner; }
  GameObject *GetOwner() const { return m_owner; }

private:
  GameObject *m_owner = nullptr;
};
