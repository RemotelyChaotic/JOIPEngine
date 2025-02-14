#ifndef CTEASESTORAGE_H
#define CTEASESTORAGE_H

#include <QJSValue>
#include <QObject>
#include <QPointer>

class CProjectSavegameManager;

class CTeaseStorageWrapper : public QObject
{
  Q_OBJECT
  Q_PROPERTY(CProjectSavegameManager* saveManager READ GetSaveManager WRITE SetSaveManager NOTIFY saveManagerChanged)

public:
  CTeaseStorageWrapper(QObject* pParent = nullptr);
  ~CTeaseStorageWrapper();

  CProjectSavegameManager* GetSaveManager() const;
  void SetSaveManager(CProjectSavegameManager* pManager);

public slots:
  void clear();
  QJSValue load(const QString& sId);
  QJSValue load(const QString& sId, const QString& sContext);
  void store(const QString& sId, const QJSValue& value);
  void store(const QString& sId, const QJSValue& value, const QString& sContext);

  void loadPersistent(const QString& sId, const QString& sContext);
  void storePersistent(const QString& sId, const QString& sContext);

signals:
  void saveManagerChanged();

private:
  QPointer<CProjectSavegameManager>          m_pSaveManager;
  std::map<QString, QJSValue>                m_storage;
};

#endif // CTEASESTORAGE_H
