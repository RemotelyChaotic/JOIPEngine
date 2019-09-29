#ifndef CBACKGROUNDWIDGET_H
#define CBACKGROUNDWIDGET_H

#include <QFutureWatcher>
#include <QWidget>
#include <memory>

struct SResource;
typedef std::shared_ptr<SResource> tspResource;

class CBackgroundWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CBackgroundWidget(QWidget* pParent = nullptr);
  ~CBackgroundWidget() override;

  void SetBackgroundColor(const QColor& color);
  void SetBackgroundTexture(const QString& sTexture);

signals:
  void SignalError(QString sError, QtMsgType type);

public slots:
  void SlotBackgroundColorChanged(QColor color);
  void SlotBackgroundTextureChanged(tspResource spResource);

protected:
  void paintEvent(QPaintEvent* pEvent) override;

protected slots:
  void SlotImageLoad(QString sPath);
  void SlotLoadFinished();

private:
  void StartImageLoad(QString sPath);


  std::unique_ptr<QFutureWatcher<void>>       m_spFutureWatcher;
  std::shared_ptr<QPixmap>                    m_spLoadedPixmap;
  mutable QMutex                              m_imageMutex;
  QFuture<void>                               m_future;
  QAtomicInt                                  m_iLoadState;

  QPixmap                                     m_backgroundPixmap;
  QColor                                      m_backgroundColor;
};

#endif // CBACKGROUNDWIDGET_H
