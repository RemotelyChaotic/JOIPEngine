#ifndef PROJECTCARDSELECTIONWIDGET_H
#define PROJECTCARDSELECTIONWIDGET_H

#include "Systems/DatabaseInterface/ProjectData.h"
#include <QPointer>
#include <QQuickWidget>
#include <QWidget>
#include <memory>
#include <vector>

class CDatabaseImageProvider;
class CDatabaseManager;
class CProjectScriptWrapper;
namespace Ui {
  class CProjectCardSelectionWidget;
}
class QResizeEvent;

class CProjectCardSelectionWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QColor selectionColor READ SelectionColor WRITE SetSelectionColor)

public:
  explicit CProjectCardSelectionWidget(QWidget* pParent = nullptr);
  ~CProjectCardSelectionWidget() override;

  void Initialize();
  void LoadProjects(EDownLoadStateFlags flags);
  void UnloadProjects();
  qint32 SelectedId() { return m_iSelectedProjectId; }
  void ShowWarning(const QString& sWarning);

  void SetSelectionColor(const QColor& color);
  const QColor& SelectionColor();
  bool IsLoaded() { return m_bLoadedQml; }

signals:
  void SingalSelected(qint32 iId);
  void SignalUnloadFinished();

protected slots:
  void on_pSearchWidget_SignalFilterChanged(const QString& sText);
  void on_pQmlWidget_statusChanged(QQuickWidget::Status);
  void on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);
  void SlotCardClicked(int iProjId);
  void SlotLoadProjectsPrivate(EDownLoadStateFlags flags);
  void SlotOverlayOpened();
  void SlotOverlayClosed();
  void SlotProjectAdded(qint32 iId);
  void SlotProjectDownloadFinished(qint32 iProjId);
  void SlotProjectDownloadProgressChanged(qint32 iProjId, qint32 iProgress);
  void SlotProjectRemoved(qint32 iId);
  void SlotResizeDone();
  void SlotUnloadFinished();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  void FinishUnloadPrivate();
  void InitQmlMain();

  std::unique_ptr<Ui::CProjectCardSelectionWidget> m_spUi;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  std::vector<QPointer<CProjectScriptWrapper>>     m_vpProjects;
  EDownLoadStateFlags                              m_flags;
  QColor                                           m_selectionColor;
  qint32                                           m_iSelectedProjectId;
  bool                                             m_bLoadedQml;
};

#endif // PROJECTCARDSELECTIONWIDGET_H
