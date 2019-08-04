#ifndef RESOURCEDISPLAYWIDGET_H
#define RESOURCEDISPLAYWIDGET_H

#include "Enums.h"
#include "Backend/Resource.h"
#include <QFutureWatcher>
#include <QMediaPlayer>
#include <QWidget>
#include <memory>

namespace Ui {
  class CResourceDisplayWidget;
}
class QMovie;

class CResourceDisplayWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CResourceDisplayWidget(QWidget* pParent = nullptr);
  ~CResourceDisplayWidget();

  void LoadResource(tspResource spResource);
  ELoadState LoadState() const { return ELoadState::_from_integral(m_iLoadState); }
  void UnloadResource();

protected slots:
  void SlotImageLoad(QString sPath);
  void SlotLoadFinished();
  void SlotStatusChanged(QMediaPlayer::MediaStatus status);
  void SlotWebLoadFinished(bool bOk);

private:
  void StartImageLoad(QString sPath);

  std::unique_ptr<Ui::CResourceDisplayWidget> m_spUi;
  std::unique_ptr<QFutureWatcher<void>>       m_spFutureWatcher;
  std::unique_ptr<QMovie>                     m_spSpinner;
  std::shared_ptr<QPixmap>                    m_spLoadedPixmap;
  mutable QMutex                              m_pixmapMutex;
  QFuture<void>                               m_future;
  tspResource                                 m_spResource;
  QAtomicInt                                  m_iLoadState;
};

#endif // RESOURCEDISPLAYWIDGET_H
