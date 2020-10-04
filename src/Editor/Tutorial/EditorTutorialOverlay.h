#ifndef CEDITORTUTORIALOVERLAY_H
#define CEDITORTUTORIALOVERLAY_H

#include "Widgets/OverlayBase.h"
#include "Enums.h"
#include <QPointer>
#include <QTimer>

class CEditorModel;
class CSettings;
class CShortcutButton;
class QGroupBox;
class QLabel;
class QPropertyAnimation;


class CEditorTutorialOverlay : public COverlayBase
{
  Q_OBJECT
  Q_PROPERTY(qint32 alpha MEMBER m_iAnimatedAlpha)

public:
  explicit CEditorTutorialOverlay(QWidget* pParent = nullptr);
  ~CEditorTutorialOverlay() override;

  void Initialize(QPointer<CEditorModel> pEditorModel);
  void NextTutorialState();
  void SetClickToAdvanceEnabled(bool bEnabled);
  void SetHighlightedWidgets(const QStringList& vsWidgetNames);
  void ShowTutorialText(EAnchors anchor, double dPosX, double dPosY, QString sText);

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;
  void SlotTriggerNextInstruction();

signals:
  void SignalOverlayNextInstructionTriggered();

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void paintEvent(QPaintEvent* pEvent) override;

protected slots:
  void SlotAlphaAnimationFinished();
  void SlotKeyBindingsChanged();
  void SlotUpdate();

private:
  void Reset();

  std::map<QWidget*, std::pair<QRect, QImage>>  m_highlightedImages;
  std::shared_ptr<CSettings>   m_spSettings;
  QPointer<QGroupBox>          m_pTutorialTextBox;
  QPointer<QLabel>             m_pTutorialTextLabel;
  QPointer<CShortcutButton>    m_pNextButton;
  QPointer<CEditorModel>       m_pEditorModel;
  QPointer<QPropertyAnimation> m_pAlphaAnimation;
  QTimer                       m_updateTimer;
  bool                         m_bInitialized;
  bool                         m_bClickToAdvanceEnabled;
  qint32                       m_iAnimatedAlpha;
};

#endif // CEDITORTUTORIALOVERLAY_H
