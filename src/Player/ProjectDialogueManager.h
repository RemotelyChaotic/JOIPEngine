#ifndef CPROJECTDIALOGueMANAGER_H
#define CPROJECTDIALOGueMANAGER_H

#include "Systems/DialogueTree.h"
#include "Systems/Project.h"

#include <QObject>
#include <QPointer>
#include <QRegularExpression>

#include <tuple>
#include <vector>

class CProjectDialogueManager : public QObject
{
  Q_OBJECT

public:
  CProjectDialogueManager();
  ~CProjectDialogueManager();

  void LoadProject(const tspProject& spProject);
  void UnloadProject();

  std::shared_ptr<CDialogueNodeDialogue> FindDialog(const QString& sId);
  std::vector<std::shared_ptr<CDialogueNodeDialogue>> FindDialogue(const QRegularExpression& rx);
  std::vector<std::shared_ptr<CDialogueNodeDialogue>> FindDialogueByTag(const QStringList& vsTags);

private:
  std::shared_ptr<CDialogueNode>                                        m_spDataRootNode;
  std::vector<std::pair<QString, std::shared_ptr<CDialogueNodeDialogue>>> m_vspDialoguesOnlyFlat;
};

//----------------------------------------------------------------------------------------
//
class CProjectDialogueManagerWrapper : public QObject
{
  Q_OBJECT

public:
  CProjectDialogueManagerWrapper(QPointer<QJSEngine> pEngine, std::weak_ptr<CProjectDialogueManager> wpInstance);
  ~CProjectDialogueManagerWrapper();

  Q_INVOKABLE QJSValue dialogue(const QString& sId);
  Q_INVOKABLE QJSValue dialogueFromRx(const QString& sId);
  Q_INVOKABLE QJSValue dialogueFromTags(const QStringList& vsId);

private:
  std::weak_ptr<CProjectDialogueManager> m_wpInstance;
  QPointer<QJSEngine> m_pEngine;
};

#endif // CPROJECTDIALOGueMANAGER_H
