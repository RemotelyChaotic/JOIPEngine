#ifndef CTEASESTORAGE_H
#define CTEASESTORAGE_H

#include <QJSValue>
#include <QObject>

class CTeaseStorageWrapper : public QObject
{
  Q_OBJECT

public:
  CTeaseStorageWrapper(QObject* pParent = nullptr);
  ~CTeaseStorageWrapper();

public slots:
  void clear();
  QJSValue load(const QString& sId);
  void store(const QString& sId, const QJSValue& value);

private:
  std::map<QString, QJSValue>                m_storage;
};

#endif // CTEASESTORAGE_H
