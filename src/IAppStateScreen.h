#ifndef IAPPSTATESCREEN_H
#define IAPPSTATESCREEN_H

#include "WindowContext.h"
#include <memory>

// define für Screen guards
#define SCREEN_INITIALIZED_GUARD \
  if (!IsInitialized()) { return; }

class IAppStateScreen
{
public:
  explicit IAppStateScreen(const std::shared_ptr<CWindowContext>& spWindowContext) :
    m_spWindowContext(spWindowContext),
    m_bInitialized(false)
  {}
  virtual ~IAppStateScreen() {}

  virtual void Load() = 0;
  virtual void Unload() = 0;

  bool IsInitialized() const { return m_bInitialized; }

protected:
  void SetInitialized(bool bInit) { m_bInitialized = bInit; }

  std::shared_ptr<CWindowContext>  m_spWindowContext;
  bool                             m_bInitialized;
};

#endif // IAPPSTATESCREEN_H
