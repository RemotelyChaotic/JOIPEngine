#ifndef SCENECARDSELECTIONWIDGET_H
#define SCENECARDSELECTIONWIDGET_H

#include <QQuickWidget>
#include <QWidget>

#include <memory>

class CDatabaseManager;
namespace Ui {
  class CSceneCardSelectionWidget;
}

class CSceneCardSelectionWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QColor selectionColor READ SelectionColor WRITE SetSelectionColor)

public:
  explicit CSceneCardSelectionWidget(QWidget* pParent = nullptr);
  ~CSceneCardSelectionWidget();

  void Initialize();
  void LoadScenes(qint32 iId, const QStringList& vsScenes);
  void UnloadScenes();
  QString SelectedScene() const { return m_sSelectedScene; }

  void SetSelectionColor(const QColor& color);
  const QColor& SelectionColor();
  bool IsLoaded() { return m_bLoadedQml; }

protected slots:
  void on_pSearchWidget_SignalFilterChanged(const QString& sText);
  void on_pQmlWidget_statusChanged(QQuickWidget::Status);
  void on_pQmlWidget_sceneGraphError(QQuickWindow::SceneGraphError error, const QString &message);
  void SlotCardClicked(QString sScene);
  void SlotLoadScenesPrivate(qint32 iId, const QStringList& vsScenes);
  void SlotOverlayOpened();
  void SlotOverlayClosed();
  void SlotResizeDone();
  void SlotUnloadFinished();

protected:
  void resizeEvent(QResizeEvent* pEvent) override;

private:
  void FinishUnloadPrivate();
  void InitQmlMain();

  std::unique_ptr<Ui::CSceneCardSelectionWidget> m_spUi;
  std::weak_ptr<CDatabaseManager>                m_wpDbManager;
  QColor                                         m_selectionColor;
  QString                                        m_sSelectedScene;
  bool                                           m_bLoadedQml;
};

#endif // SCENECARDSELECTIONWIDGET_H
