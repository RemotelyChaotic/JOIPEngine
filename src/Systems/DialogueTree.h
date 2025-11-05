#ifndef CDIALOGUENODE_H
#define CDIALOGUENODE_H

#include "Database/Resource.h"
#include "Database/Tag.h"

#include <enum.h>

#include <QString>

#include <memory>
#include <vector>

BETTER_ENUM(EDialogueTreeNodeType, qint32,
            eRoot            = 0,
            eCategory        = 1,
            eDialogue        = 2,
            eDialogueFragment= 3);

namespace dialogue_tree
{
  [[maybe_unused]] const char c_sRootNodeName[] = "<Root>";
}

struct CDialogueNode
{
  CDialogueNode();
  virtual ~CDialogueNode();

  std::shared_ptr<CDialogueNode> Clone();
  void CopyFrom(const std::shared_ptr<CDialogueNode>& spNode);

private:
  virtual std::shared_ptr<CDialogueNode> CloneImpl();
  virtual void CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode);

public:
  std::weak_ptr<CDialogueNode>                m_wpParent;
  std::vector<std::shared_ptr<CDialogueNode>> m_vspChildren;

  QString                                   m_sFileId;
  EDialogueTreeNodeType                     m_type;
  QString                                   m_sName;
  bool                                      m_bReadOnly;
};

struct CDialogueNodeCategory : public CDialogueNode
{
  CDialogueNodeCategory();
  ~CDialogueNodeCategory() override;

private:
  std::shared_ptr<CDialogueNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode) override;
};

struct CDialogueNodeDialogue : public CDialogueNode
{
  CDialogueNodeDialogue();
  ~CDialogueNodeDialogue() override;

private:
  std::shared_ptr<CDialogueNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode) override;

public:
  tspTagMap                                 m_tags;
  bool                                      m_bHasCondition = false;
};

struct CDialogueData : public CDialogueNode
{
  CDialogueData();
  ~CDialogueData() override;

private:
  std::shared_ptr<CDialogueNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode) override;

public:
  QString                                   m_sCondition;
  QString                                   m_sString;
  QString                                   m_sSoundResource;
  qint64                                    m_iWaitTimeMs = -1; // -1 determins minimum of audio and length
  bool                                      m_bSkipable;
};

typedef std::shared_ptr<struct SProject> tspProject;

namespace dialogue_tree
{
  QString EnsureUniqueName(const QString& sStr, const std::shared_ptr<CDialogueNode>& spParent,
                           const std::shared_ptr<CDialogueNode>& spExcept);

  std::shared_ptr<CDialogueNode> DeserializeNode(const QByteArray& arr, const tspProject& spProject);
  std::shared_ptr<CDialogueNode> LoadDialogues(const std::vector<tspResource>& vsFiles, bool* bpErrors = nullptr);
  std::shared_ptr<CDialogueNode> LoadDialoguesFromSource(const std::vector<QUrl>& vsFiles,
                                                     const tspProject& spProject,
                                                     bool* bpErrors = nullptr);
  void SaveDialogues(const std::shared_ptr<CDialogueNode>& spDialogueNodeTree, const tspProject& spProject);
  QByteArray SerializeNode(const std::shared_ptr<CDialogueNode>& spNode);
}

#endif // CDIALOGUENODE_H
