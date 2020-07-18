#ifndef PROJECTCARDSELECTIONWIDGET_H
#define PROJECTCARDSELECTIONWIDGET_H

#include <QPointer>
#include <QQuickWidget>
#include <QWidget>
#include <memory>
#include <vector>

class CDatabaseImageProvider;
class CDatabaseManager;
class CProject;
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
  void LoadProjects();
  void UnloadProjects();
  qint32 SelectedId() { return m_iSelectedProjectId; }

  void SetSelectionColor(const QColor& color);
  const QColor& SelectionColor();

protected slots:
  void on_pSearchWidget_SignalFilterChanged(const QString& sText);
  void on_pQmlWidget_statusChanged(QQuickWidget::Status);
  void on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);
  void SlotCardClicked(int iProjId);
  void SlotOverlayOpened();
  void SlotOverlayClosed();
  void SlotResizeDone();
  void SlotTriedToLoadMovie(const QString& sMovie);

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  void InitQmlMain();

  std::unique_ptr<Ui::CProjectCardSelectionWidget> m_spUi;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  std::vector<QPointer<CProject>>                  m_vpProjects;
  QColor                                           m_selectionColor;
  qint32                                           m_iSelectedProjectId;
  bool                                             m_bInitialized;
};

#endif // PROJECTCARDSELECTIONWIDGET_H
