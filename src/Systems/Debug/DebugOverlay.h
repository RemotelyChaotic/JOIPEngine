#ifndef CDEBUGOVERLAY_H
#define CDEBUGOVERLAY_H

#include "ICustomMessageHandler.h"

#include "Widgets/OverlayBase.h"

#include <QLabel>
#include <QPointer>

#include <memory>

class CDebugInterface;
class CHighlightedSearchableTextEdit;
class CSettings;
class CTextEditZoomEnabler;
namespace Ui {
  class CDebugOverlay;
}

class CDebugOverlay : public COverlayBase, public ICustomMessageHandler
{
  Q_OBJECT

public:
  explicit CDebugOverlay(QWidget* pParent = nullptr);
  ~CDebugOverlay() override;

  static CDebugOverlay* GetInstance();

  void Climb() override;
  void Hide() override;
  void Resize() override;
  void Show() override;

  bool MessageImpl(QtMsgType type, const QMessageLogContext& context,
                   const QString& sMsg) override;

private slots:
  void on_CloseButton_clicked();
  void on_CliButton_clicked();
  void on_LogInput_editingFinished();
  void SlotDebugOverlayEnabledChanged();
  void SlotKeyBindingsChanged();
  void SlotMessageImpl(QtMsgType type, const QString& sMsg);

private:
  static CDebugOverlay*              m_pInstance;

  std::unique_ptr<Ui::CDebugOverlay> m_spUi;
  std::shared_ptr<CSettings>         m_spSettings;
  std::weak_ptr<CDebugInterface>     m_wpDebugInterface;
  QPointer<CHighlightedSearchableTextEdit> m_pHighlightedSearchableEdit;
  QPointer<CTextEditZoomEnabler>     m_pZoomEnabler;
  QPointer<QAction>                  m_pActionToggle;
};

#endif // CDEBUGOVERLAY_H
