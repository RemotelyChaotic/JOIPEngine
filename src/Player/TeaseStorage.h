#ifndef CTEASESTORAGE_H
#define CTEASESTORAGE_H

#include <QJSValue>
#include <QObject>

class CTeaseStorage : public QObject
{
  Q_OBJECT

public:
  CTeaseStorage(QObject* pParent = nullptr);
  ~CTeaseStorage();

public slots:
  void clear();
  QJSValue load(const QString& sId);
  void store(const QString& sId, const QJSValue& value);

private:
  std::map<QString, QJSValue>                m_storage;
};

#endif // CTEASESTORAGE_H
