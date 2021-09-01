#ifndef PROJECTRUNNER_H
#define PROJECTRUNNER_H

#include <QObject>
#include <memory>
#include <random>

class CDatabaseManager;
class CFlowScene;
namespace QtNodes {
  class Node;
}
struct SProject;
struct SScene;
typedef std::shared_ptr<SProject> tspProject;
typedef std::shared_ptr<SScene> tspScene;

class CProjectRunner : public QObject
{
  Q_OBJECT

public:
  explicit CProjectRunner(QObject* pParent = nullptr);
  ~CProjectRunner();

  void LoadProject(tspProject spProject, const QString sStartScene);
  void UnloadProject();

  tspScene NextScene(const QString sName);
  QStringList PossibleScenes();
  void ResolveFindScenes(const QString sName);
  void ResolveScenes();

signals:
  void SignalError(QString sError, QtMsgType type);

private:
  bool LoadFlowScene();
  void ResolveNextPossibleNodes(QtNodes::Node* pNode, std::vector<std::pair<QString, QtNodes::Node*>>& vpRet);
  bool ResolveNextScene();
  bool ResolveStart(const QString sStartScene);

private slots:
  void SlotNodeCreated(QtNodes::Node &n);

private:
  tspProject                        m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>   m_wpDbManager;
  std::map<QString, QtNodes::Node*> m_nodeMap;
  CFlowScene*                       m_pFlowScene;
  QtNodes::Node*                    m_pCurrentNode;
  std::mt19937                      m_generator;
};

#endif // PROJECTRUNNER_H
