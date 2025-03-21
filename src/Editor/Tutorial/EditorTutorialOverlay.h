#ifndef CEDITORTUTORIALOVERLAY_H
#define CEDITORTUTORIALOVERLAY_H

#include "Widgets/OverlayBase.h"
#include "Enums.h"
#include <QPointer>
#include <QTimer>

class CEditorModel;
class CSettings;
class CShortcutButton;
class QDialogButtonBox;
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

  static QPointer<CEditorTutorialOverlay> Instance();

  void Initialize(QPointer<CEditorModel> pEditorModel);

public slots:
  void NextTutorialState();
  void SetClickToAdvanceEnabled(bool bEnabled);
  void SetClickFilterWidgets(const QStringList& vsWidgetNames, bool bTriggerNextAfterClick = true);
  void SetHighlightedWidgets(const QStringList& vsWidgetNames, bool bAlwaysOnTop);
  void SetMouseTransparecny(bool bEnabled);
  void ShowTutorialText(int anchor, double dPosX, double dPosY,
                        bool bHideButtons, QString sText);

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
  QPointer<QWidget> ClickedFilterChild(const QPoint& locClicked);
  QList<QPointer<QWidget>> FindWidgetsByName(const QStringList& vsWidgetNames);
  QPoint MapPosToGlobal(const QPointer<QWidget> pWidget, const QPoint& widgetPos);
  void Reset();

  static CEditorTutorialOverlay*                m_pInstance;
  std::map<QWidget*, std::pair<QRect, QImage>>  m_highlightedImages;
  QList<QPointer<QWidget>>                      m_vpClickFilterWidgets;
  std::shared_ptr<CSettings>                    m_spSettings;
  QPointer<COverlayBase>                        m_pTutorialTextBox;
  QPointer<QLabel>                              m_pTutorialTextLabel;
  QPointer<QDialogButtonBox>                    m_pTutorialpButtonBox;
  QPointer<CShortcutButton>                     m_pNextButton;
  QPointer<CEditorModel>                        m_pEditorModel;
  QPointer<QPropertyAnimation>                  m_pAlphaAnimation;
  QTimer                                        m_updateTimer;
  bool                                          m_bInitialized;
  bool                                          m_bClickToAdvanceEnabled;
  bool                                          m_bTriggerNextAfterWidgetClick;
  qint32                                        m_iAnimatedAlpha;
};

#endif // CEDITORTUTORIALOVERLAY_H
