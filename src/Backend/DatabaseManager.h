#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "ThreadedSystem.h"

class CSettings;

class CDatabaseManager : public CThreadedObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CDatabaseManager)

public:
  CDatabaseManager();
  ~CDatabaseManager() override;

public slots:
  void Initialize() override;
  void Deinitialize() override;

private slots:
  void SlotContentFolderChanged();

private:
  std::shared_ptr<CSettings>    m_spSettings;
};

#endif // DATABASEMANAGER_H
