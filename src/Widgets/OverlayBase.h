#ifndef OVERLAYBASE_H
#define OVERLAYBASE_H

#include <QPointer>
#include <QFrame>
#include <QMainWindow>
#include <memory>

class COverlayManager;

class COverlayBase : public QFrame
{
  Q_OBJECT

public:
  explicit COverlayBase(qint32 iZOrder, QWidget* pParent = nullptr);
  ~COverlayBase() override;

public slots:
  virtual void Climb() = 0;
  virtual void Hide();
  virtual void Resize();
  virtual void Show();

  bool IsInPlace() const;
  qint32 ZOrder() const;

protected:
  virtual void RebuildZOrder();

  void ClimbToCentralWidget();
  void ClimbToFirstInstanceOf(const QString& sClassName, bool bToParentOfFound);
  void ClimbToTop();

  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
  void FinishClimb(QWidget* pParentW);
  QMainWindow* FindMainWindow();

protected:
  std::weak_ptr<COverlayManager>                    m_wpOverlayManager;
  QPointer<QWidget>                                 m_pOriginalParent;
  QPointer<QWidget>                                 m_pTargetWidget;
  qint32                                            m_iZOrder;
  bool                                              m_bReachedDestination;
  bool                                              m_bShowCalled;
};

#endif // OVERLAYBASE_H
