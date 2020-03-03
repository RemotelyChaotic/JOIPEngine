#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QObject>
#include <QPointer>
#include <memory>

class CDatabaseManager;
class CResourceTreeItemModel;
class CSettings;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;


class CEditorModel : public QObject
{
  Q_OBJECT
public:
  explicit CEditorModel(QObject* pParent = nullptr);
  ~CEditorModel();

  const tspProject& CurrentProject() const;
  CResourceTreeItemModel* ResourceTreeModel() const;

  void AddFilesToProjectResources(QPointer<QWidget> pParentForDialog,
    const QStringList& vsFiles, const QStringList& imageFormatsList,
    const QStringList& videoFormatsList, const QStringList& audioFormatsList,
    const QStringList& otherFormatsList);
  void InitNewProject(const QString& sNewProjectName);
  void LoadProject(qint32 iId);
  QString RenameProject(const QString& sNewProjectName);
  void UnloadProject();
  void SerializeProject();

signals:
  void SignalProjectEdited();

private:
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitializingNewProject;
};

#endif // EDITORMODEL_H
