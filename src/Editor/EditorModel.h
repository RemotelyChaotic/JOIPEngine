#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QObject>
#include <QPointer>
#include <memory>

class CDatabaseManager;
class CKinkTreeModel;
class CResourceTreeItemModel;
class CScriptEditorModel;
class CSettings;
namespace QtNodes {
  class FlowScene;
  class Node;
}
struct SScene;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;

class CEditorModel : public QObject
{
  Q_OBJECT
public:
  explicit CEditorModel(QWidget* pParent = nullptr);
  ~CEditorModel();

  const tspProject& CurrentProject() const;
  QtNodes::FlowScene* FlowSceneModel() const;
  CKinkTreeModel* KinkTreeModel() const;
  CResourceTreeItemModel* ResourceTreeModel() const;
  CScriptEditorModel* ScriptEditorModel() const;

  void AddFilesToProjectResources(QPointer<QWidget> pParentForDialog,
                                  const QStringList& vsFiles,
                                  const QStringList& imageFormatsList,
                                  const QStringList& videoFormatsList,
                                  const QStringList& audioFormatsList,
                                  const QStringList& otherFormatsList,
                                  const QStringList& scriptFormatsList,
                                  const QStringList& databaseFormatsList);
  void AddNewScriptFileToScene(QPointer<QWidget> pParentForDialog,
                               tspScene spScene);

  void InitNewProject(const QString& sNewProjectName);
  void LoadProject(qint32 iId);
  QString RenameProject(const QString& sNewProjectName);
  void SaveProject();
  void UnloadProject();
  void SerializeProject();

public slots:
  void SlotNodeCreated(QtNodes::Node &n);
  void SlotNodeDeleted(QtNodes::Node &n);

signals:
  void SignalProjectEdited();

private:
  std::unique_ptr<CKinkTreeModel>                             m_spKinkTreeModel;
  std::unique_ptr<CResourceTreeItemModel>                     m_spResourceTreeModel;
  std::unique_ptr<CScriptEditorModel>                         m_spScriptEditorModel;
  std::unique_ptr<QtNodes::FlowScene>                         m_spFlowSceneModel;
  std::shared_ptr<CSettings>                                  m_spSettings;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  QPointer<QWidget>                                           m_pParentWidget;
  bool                                                        m_bInitializingNewProject;
};

#endif // EDITORMODEL_H
