#ifndef CDIALOGNODE_H
#define CDIALOGNODE_H

#include "Resource.h"
#include "Tag.h"

#include <enum.h>

#include <QString>

#include <memory>
#include <vector>

BETTER_ENUM(EDialogTreeNodeType, qint32,
            eRoot            = 0,
            eCategory        = 1,
            eDialog          = 2,
            eDialogFragment  = 3);

namespace dialog_tree
{
  [[maybe_unused]] const char c_sRootNodeName[] = "<Root>";
}

struct CDialogNode
{
  CDialogNode();
  virtual ~CDialogNode();

  std::shared_ptr<CDialogNode> Clone();
  void CopyFrom(const std::shared_ptr<CDialogNode>& spNode);

private:
  virtual std::shared_ptr<CDialogNode> CloneImpl();
  virtual void CopyFromImpl(const std::shared_ptr<CDialogNode>& spNode);

public:
  std::weak_ptr<CDialogNode>                m_wpParent;
  std::vector<std::shared_ptr<CDialogNode>> m_vspChildren;

  QString                                   m_sFileId;
  EDialogTreeNodeType                       m_type;
  QString                                   m_sName;
  bool                                      m_bReadOnly;
};

struct CDialogNodeCategory : public CDialogNode
{
  CDialogNodeCategory();
  ~CDialogNodeCategory() override;

private:
  std::shared_ptr<CDialogNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogNode>& spNode) override;
};

struct CDialogNodeDialog : public CDialogNode
{
  CDialogNodeDialog();
  ~CDialogNodeDialog() override;

private:
  std::shared_ptr<CDialogNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogNode>& spNode) override;

public:
  tspTagMap                                 m_tags;
  bool                                      m_bHasCondition = false;
};

struct CDialogData : public CDialogNode
{
  CDialogData();
  ~CDialogData() override;

private:
  std::shared_ptr<CDialogNode> CloneImpl() override;
  void CopyFromImpl(const std::shared_ptr<CDialogNode>& spNode) override;

public:
  QString                                   m_sCondition;
  QString                                   m_sString;
  QString                                   m_sSoundResource;
  qint64                                    m_iWaitTimeMs = -1; // -1 determins minimum of audio and length
  bool                                      m_bSkipable;
};

typedef std::shared_ptr<struct SProject> tspProject;

namespace dialog_tree
{
  QString EnsureUniqueName(const QString& sStr, const std::shared_ptr<CDialogNode>& spParent,
                           const std::shared_ptr<CDialogNode>& spExcept);

  std::shared_ptr<CDialogNode> DeserializeNode(const QByteArray& arr, const tspProject& spProject);
  std::shared_ptr<CDialogNode> LoadDialogs(const std::vector<tspResource>& vsFiles, bool* bpErrors = nullptr);
  std::shared_ptr<CDialogNode> LoadDialogsFromSource(const std::vector<QUrl>& vsFiles,
                                                     const tspProject& spProject,
                                                     bool* bpErrors = nullptr);
  void SaveDialogs(const std::shared_ptr<CDialogNode>& spDialogNodeTree, const tspProject& spProject);
  QByteArray SerializeNode(const std::shared_ptr<CDialogNode>& spNode);
}

#endif // CDIALOGNODE_H
