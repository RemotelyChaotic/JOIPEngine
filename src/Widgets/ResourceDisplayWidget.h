#ifndef RESOURCEDISPLAYWIDGET_H
#define RESOURCEDISPLAYWIDGET_H

#include "Enums.h"
#include "Backend/Resource.h"
#include <QtAV/QtAV_Global.h>
#include <QFutureWatcher>
#include <QMediaPlayer>
#include <QWidget>
#include <memory>

class CSettings;
namespace Ui {
  class CResourceDisplayWidget;
}
class QMovie;

class CResourceDisplayWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CResourceDisplayWidget(QWidget* pParent = nullptr);
  ~CResourceDisplayWidget() override;

  void LoadResource(tspResource spResource);
  ELoadState LoadState() const { return ELoadState::_from_integral(m_iLoadState); }
  void UnloadResource();

  qint32 ProjectId() { return m_iProjectId; }
  EResourceType ResourceType();
  bool IsRunning();

  void SetProjectId(qint32 iId) { m_iProjectId = iId; }
  void SetMargins(qint32 iLeft, qint32 iTop, qint32 iRight, qint32 iBottom);

public slots:
  void SlotPlayPause();
  void SlotStop();
  void SlotSetSliderVisible(bool bVisible);

signals:
  void OnClick();
  void SignalLoadFinished();
  void SignalPlaybackFinished();

protected:
  void mousePressEvent(QMouseEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvent) override;

protected slots:
  void SlotImageLoad(QString sPath);
  void SlotLoadFinished();
  void SlotMutedChanged();
  void SlotStatusChanged(QtAV::MediaStatus status);
  void SlotVolumeChanged();
  void SlotWebLoadFinished(bool bOk);

private:
  void StartImageLoad(QString sPath);

  std::unique_ptr<Ui::CResourceDisplayWidget> m_spUi;
  std::unique_ptr<QFutureWatcher<void>>       m_spFutureWatcher;
  std::unique_ptr<QMovie>                     m_spSpinner;
  std::shared_ptr<CSettings>                  m_spSettings;
  std::shared_ptr<QMovie>                     m_spLoadedMovie;
  std::shared_ptr<QPixmap>                    m_spLoadedPixmap;
  mutable QMutex                              m_imageMutex;
  QFuture<void>                               m_future;
  tspResource                                 m_spResource;
  QAtomicInt                                  m_iLoadState;
  QAtomicInt                                  m_iProjectId;
};

#endif // RESOURCEDISPLAYWIDGET_H
