#ifndef PROJECTRUNNER_H
#define PROJECTRUNNER_H

#include <QObject>
#include <QUuid>

#include <memory>
#include <optional>
#include <random>
#include <set>
#include <stack>
#include <variant>

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
class IResolverDebugger
{
public:
  struct NodeData
  {
    bool bEnabled = true;
    bool bSelection = false;
    QString sLabel;
    qint32 iPortIndex = -1;
  };

  virtual std::vector<NodeData*> ChildBlocks(const QtNodes::Node* pNode) const = 0;
  virtual NodeData* DataBlock(const QtNodes::Node* pNode) const = 0;
  virtual void Error(const QString& sError, QtMsgType type) = 0;
  virtual void PopFlow() = 0;
  virtual void PushFlow(const QString& sNewFlow) = 0;
  virtual void PushNode(const QtNodes::Node* const pParent, QtNodes::Node* const pNext,
                        NodeData data) = 0;
  virtual void ResolveTo(const QtNodes::Node* const pNode) = 0;
  virtual void SetCurrentNode(QtNodes::Node* const pNode) = 0;

protected:
  IResolverDebugger();
  virtual ~IResolverDebugger();
};

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
struct SSceneFlowBlock
{
  QString                                     m_sName;
  CFlowScene*                                 m_pFlowScene = nullptr;
  std::vector<NodeResolveReslt>               m_resolveResult;
  std::map<QString, QtNodes::Node*>           m_nodeMap;
  QtNodes::Node*                              m_pCurrentNode = nullptr;
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

  void AttatchDebugger(const std::weak_ptr<IResolverDebugger>& wpDebugger);
  void LoadProject(tspProject spProject, const QUuid& nodeId);
  void LoadProject(tspProject spProject, const QString& sStartScene);
  void LoadProject(tspProject spProject, const tspScene& spStartScene);
  void UnloadProject();

  QStringList AllScenes() const;
  tspScene CurrentScene() const;
  void DisableScene(const QString& sScene);
  void EnableScene(const QString& sScene);
  bool IsSceneEnabled(const QString& sScene) const;
  tspScene NextScene(const QString sName, bool* bEnd, QStringList* pvsPossibleScenes,
                     std::optional<QString>* pUnresolvedData);
  QStringList PossibleScenes(std::optional<QString>* unresolvedData);
  void ResolveFindScenes(std::variant<QString, QUuid> sceneIdentifier, bool bFindStart = false);
  void ResolvePossibleScenes(const QStringList vsNames, qint32 iIndex);
  void ResolveScenes();

signals:
  void SignalChangeSceneRequest(const QString& sScene, bool bIgnoreMissing);
  void SignalError(QString sError, QtMsgType type);

private:
  void Error(const QString& sError, QtMsgType type);
  bool GenerateNodesFromResolved();
  bool PushFlowBlock(const QString& sName);
  bool PopFlowBlock();
  bool Setup(tspProject spProject, const std::variant<QString, QUuid>& start, bool bInjectedScene);
  bool LoadFlowScenes();
  bool ResolveNextScene();
  bool ResolveStart(const std::variant<QString, QUuid>& start);

private slots:
  void SlotNodeCreated(QtNodes::Node &n);

private:
  std::shared_ptr<QtNodes::DataModelRegistry> m_spNodeModelRegistry;
  tspProject                                  m_spCurrentProject;
  tspScene                                    m_spCurrentScene;
  tspScene                                    m_spInjectedScene;
  std::weak_ptr<IResolverDebugger>            m_wpDebugger;
  std::weak_ptr<CDatabaseManager>             m_wpDbManager;
  std::vector<SSceneFlowBlock>                m_vLoadedSceneBlocks;
  std::stack<SSceneFlowBlock>                 m_flowStack;
  std::set<QString>                           m_disabledScenes;
  std::mt19937                                m_generator;
};

#endif // PROJECTRUNNER_H
