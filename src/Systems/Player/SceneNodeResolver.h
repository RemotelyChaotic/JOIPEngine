#ifndef PROJECTRUNNER_H
#define PROJECTRUNNER_H

#include <QObject>
#include <memory>
#include <optional>
#include <random>
#include <set>

class CDatabaseManager;
class CFlowScene;
namespace QtNodes {
  class DataModelRegistry;
  class Node;
}
struct SProject;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;

//----------------------------------------------------------------------------------------
//
struct NodeResolveReslt
{
  QString m_sLabel;
  QtNodes::Node* m_pNode;
  qint32 m_iDepth;
  bool m_bNeedsUserResolvement = false;
  QString m_sResolvementData;
};

//----------------------------------------------------------------------------------------
//
class CSceneNodeResolver : public QObject
{
  Q_OBJECT

public:
  explicit CSceneNodeResolver(std::shared_ptr<QtNodes::DataModelRegistry> spRegistry,
                              QObject* pParent = nullptr);
  ~CSceneNodeResolver();

  static bool MightBeRegexScene(const QString& sName);

  void LoadProject(tspProject spProject, const QString sStartScene);
  void LoadProject(tspProject spProject, tspScene spStartScene);
  void UnloadProject();

  tspScene CurrentScene() const;
  void DisableScene(const QString& sScene);
  void EnableScene(const QString& sScene);
  bool IsSceneEnabled(const QString& sScene) const;
  tspScene NextScene(const QString sName, bool* bEnd);
  QStringList PossibleScenes(std::optional<QString>* unresolvedData);
  void ResolveFindScenes(const QString sName);
  void ResolvePossibleScenes(const QStringList vsNames, qint32 iIndex);
  void ResolveScenes();

signals:
  void SignalChangeSceneRequest(const QString& sScene);
  void SignalError(QString sError, QtMsgType type);

private:
  bool GenerateNodesFromResolved();
  bool Setup(tspProject spProject, const QString sStartScene);
  bool LoadFlowScene();
  bool ResolveNextScene();
  bool ResolveStart(const QString sStartScene);

private slots:
  void SlotNodeCreated(QtNodes::Node &n);

private:
  std::shared_ptr<QtNodes::DataModelRegistry> m_spNodeModelRegistry;
  tspProject                                  m_spCurrentProject;
  tspScene                                    m_spCurrentScene;
  tspScene                                    m_spInjectedScene;
  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  std::vector<NodeResolveReslt>               m_resolveResult;
  std::map<QString, QtNodes::Node*>           m_nodeMap;
  std::set<QString>                           m_disabledScenes;
  CFlowScene*                                 m_pFlowScene;
  QtNodes::Node*                              m_pCurrentNode;
  std::mt19937                                m_generator;
};

#endif // PROJECTRUNNER_H
