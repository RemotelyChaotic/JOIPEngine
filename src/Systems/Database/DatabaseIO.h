#ifndef CDATABASELOADER_H
#define CDATABASELOADER_H

#include <QAtomicInt>
#include <QUrl>
#include <memory>

class CDatabaseData;
class CDatabaseManager;
typedef std::shared_ptr<struct SProject>       tspProject;
typedef std::shared_ptr<struct SResource>      tspResource;

class CDatabaseIO
{
public:
  static std::unique_ptr<CDatabaseIO> CreateDatabaseIO(CDatabaseManager* pManager,
                                                       std::shared_ptr<CDatabaseData> spData);
  static bool LoadBundle(tspProject& spProject, const QString& sBundle);
  static bool LoadProject(tspProject& spProject);
  static bool SetProjectEditing(tspProject& spProject, bool bEnabled);
  static bool UnloadBundle(tspProject& spProject, const QString& sBundle);
  static bool UnloadProject(tspProject& spProject);

  static void LoadResource(tspResource& spRes);
  static void UnloadResource(tspResource& spRes);
  static bool MountProject(tspProject& spProject);
  static bool UnmountProject(tspProject& spProject);

  CDatabaseIO(CDatabaseManager* pManager,
              std::shared_ptr<CDatabaseData> spData);
  virtual ~CDatabaseIO() {}

  bool AddResourceArchive(tspProject& spProj, const QUrl& sPath);
  bool DeserializeProject(tspProject& spProject);
  void LoadDatabase();
  bool PrepareProject(tspProject& spProject);
  bool SerializeProject(tspProject& spProject, bool bForceWriting);

  bool IsDbLoaded() const { return m_bLoadedDb == 1; }
  void SetDbLoaded(bool bLoaded);

protected:
  virtual bool DeserializeProjectImpl(tspProject& spProject) = 0;
  virtual void LoadProjects() = 0;
  virtual void LoadKinks() = 0;
  virtual bool PrepareProjectImpl(tspProject& spProject) = 0;
  virtual bool SerializeProjectImpl(tspProject& spProject, bool bForceWriting) = 0;

  CDatabaseManager*              m_pManager;
  std::shared_ptr<CDatabaseData> m_spData;
  QAtomicInt                     m_bLoadedDb;
};

#endif // CDATABASELOADER_H
