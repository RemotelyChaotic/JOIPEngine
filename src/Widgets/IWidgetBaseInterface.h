#ifndef IWIDGETBASEINTERFACE_H
#define IWIDGETBASEINTERFACE_H

// define für Screen guards
#define WIDGET_INITIALIZED_GUARD \
  if (!IsInitialized()) { return; }

class IWidgetBaseInterface
{
public:
  IWidgetBaseInterface() :
    m_bInitialized(false) {}
  virtual ~IWidgetBaseInterface() {}

  virtual void Initialize() = 0;

  bool IsInitialized() const { return m_bInitialized; }

protected:
  void SetInitialized(bool bInitialized) { m_bInitialized = bInitialized; }

  bool          m_bInitialized;
};

#endif // IWIDGETBASEINTERFACE_H
