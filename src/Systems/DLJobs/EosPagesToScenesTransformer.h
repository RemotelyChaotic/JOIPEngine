#ifndef CEOSPAGESTOSCENESTRANSFORMER_H
#define CEOSPAGESTOSCENESTRANSFORMER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QPointF>
#include <QUuid>
#include <memory>
#include <vector>

class CFlowScene;
namespace QtNodes {
  class Node;
}
typedef std::shared_ptr<struct SProject> tspProject;
typedef std::shared_ptr<struct SScene> tspScene;

class CEosPagesToScenesTransformer
{
public:
  struct SPageScene
  {
    QString       m_sName;
    QJsonDocument m_pageScript;
  };

  CEosPagesToScenesTransformer(const QJsonDocument& script);
  ~CEosPagesToScenesTransformer();

  tspScene AddPageToScene(const qint32 iProjectId,
                          tspProject spProject,
                          const SPageScene& page);
  bool CollectScenes(QString* psError);
  QByteArray CompileScenes();

  std::vector<SPageScene> m_vPages;

private:
  QUuid CreateNodeAndAdd(const QString sName, QPointF pos);
  QtNodes::Node* GetNode(QUuid uuid);

  std::unique_ptr<CFlowScene> m_spScene;
  const QJsonDocument&        m_script;
  QUuid                       m_startId;
  QUuid                       m_lastNode;
  QUuid                       m_endId;
};

#endif // CEOSPAGESTOSCENESTRANSFORMER_H
