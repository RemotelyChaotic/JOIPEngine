#ifndef OVERLAYBASE_H
#define OVERLAYBASE_H

#include <QPointer>
#include <QFrame>
#include <memory>

class COverlayBase : public QFrame
{
  Q_OBJECT

public:
  explicit COverlayBase(QWidget* pParent = nullptr);
  ~COverlayBase() override;

public:
  virtual void Hide();
  virtual void Resize();
  virtual void Show();

protected:
  virtual void ClimbToTop();

  bool event(QEvent* pEvent) override;

  QPointer<QWidget>                 m_pTarget;
};

#endif // OVERLAYBASE_H
