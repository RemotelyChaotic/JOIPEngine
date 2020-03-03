#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QObject>
#include <memory>

class CDatabaseManager;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorModel : public QObject
{
  Q_OBJECT
public:
  explicit CEditorModel(QObject* pParent = nullptr);
  ~CEditorModel();

  const tspProject& CurrentProject() const;
  void InitNewProject(const QString& sNewProjectName);
  void LoadProject(qint32 iId);
  QString RenameProject(const QString& sNewProjectName);
  void UnloadProject();
  void SerializeProject();

private:
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  bool                                                        m_bInitializingNewProject;
};

#endif // EDITORMODEL_H
