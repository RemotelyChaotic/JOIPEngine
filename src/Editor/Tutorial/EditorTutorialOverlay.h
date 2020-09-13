#ifndef CEDITORTUTORIALOVERLAY_H
#define CEDITORTUTORIALOVERLAY_H

#include "Widgets/OverlayBase.h"
#include <QPointer>
#include <QTimer>

class CEditorModel;
class QPropertyAnimation;

class CEditorTutorialOverlay : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 alpha MEMBER m_iAnimatedAlpha)

public:
  explicit CEditorTutorialOverlay(QWidget* pParent = nullptr);
  ~CEditorTutorialOverlay() override;

  void Initialize(QPointer<CEditorModel> pEditorModel);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;

protected:
  void paintEvent(QPaintEvent* pEvent) override;

protected slots:
  void SlotAlphaAnimationFinished();
  void SlotUpdate();

private:
  void Reset();

  QPointer<CEditorModel>       m_pEditorModel;
  QPointer<QPropertyAnimation> m_pAlphaAnimation;
  QTimer                       m_updateTimer;
  bool                         m_bInitialized;
  qint32                       m_iAnimatedAlpha;
};

#endif // CEDITORTUTORIALOVERLAY_H
