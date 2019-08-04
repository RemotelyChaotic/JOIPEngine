#ifndef PROJECTCARDWIDGET_H
#define PROJECTCARDWIDGET_H

#include "Backend/Project.h"
#include "Enums.h"
#include <QFutureWatcher>
#include <QWidget>
#include <memory>

namespace Ui {
  class CProjectCardWidget;
}
class QMovie;

//----------------------------------------------------------------------------------------
//
class CProjectCardWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CProjectCardWidget(std::shared_ptr<QMovie> spSpinner, QWidget* pParent = nullptr);
  ~CProjectCardWidget() override;

  void Initialize(const tspProject& spProject);
  ELoadState LoadState() const { return ELoadState::_from_integral(m_iLoadState); }
  qint32 ProjectId() const { return m_iProjectId; }

signals:
  void Clicked();

protected:
  void mousePressEvent(QMouseEvent* pEvent) override;

private slots:
  void Load(QString sPath);
  void LoadFinished();

private:
  void StartLoad(QString sPath);

  std::unique_ptr<Ui::CProjectCardWidget> m_spUi;
  std::unique_ptr<QFutureWatcher<void>>   m_spFutureWatcher;
  std::shared_ptr<QMovie>                 m_spSpinner;
  std::shared_ptr<QPixmap>                m_spLoadedPixmap;
  mutable QMutex                          m_pixmapMutex;
  QFuture<void>                           m_future;
  QAtomicInt                              m_iLoadState;
  qint32                                  m_iProjectId;
};

#endif // PROJECTCARDWIDGET_H
