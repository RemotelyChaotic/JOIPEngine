#ifndef CDATABASEIMAGEPROVIDER_H
#define CDATABASEIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <memory>

class CDatabaseManager;

class CDatabaseImageProvider : public QObject, public QQuickImageProvider
{
  Q_OBJECT

public:
  explicit CDatabaseImageProvider(const std::weak_ptr<CDatabaseManager>& wpDatabase);
  ~CDatabaseImageProvider();

  QImage requestImage(const QString& id, QSize* pSize, const QSize& requestedSize) override;

signals:
  void SignalTriedToLoadMovie(const QString& sPath);

private:
  QImage LoadImage(const QString& sPath);

  std::weak_ptr<CDatabaseManager> m_wpDatabase;
};

#endif // CDATABASEIMAGEPROVIDER_H
