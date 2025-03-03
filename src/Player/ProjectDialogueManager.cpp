#include "ProjectDialogueManager.h"

#include "Systems/Script/ScriptDbWrappers.h"

#include <QFileInfo>

CProjectDialogueManager::CProjectDialogueManager()
{

}

CProjectDialogueManager::~CProjectDialogueManager()
{

}

//----------------------------------------------------------------------------------------
//
namespace
{
  void ParseTreeItems(const std::shared_ptr<CDialogueNode>& spNode,
                      std::vector<std::pair<QString, std::shared_ptr<CDialogueNodeDialogue>>>* pvspDataFlat)
  {
    for (const auto& spChildNode : spNode->m_vspChildren)
    {
      if (EDialogueTreeNodeType::eDialogue == spChildNode->m_type._to_integral())
      {
        pvspDataFlat->push_back({spChildNode->m_sName,
                                 std::dynamic_pointer_cast<CDialogueNodeDialogue>(spChildNode)});
      }
      ParseTreeItems(spChildNode, pvspDataFlat);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDialogueManager::LoadProject(const tspProject& spProject)
{
  std::vector<tspResource> vsResFiles;
  {
    QReadLocker lock(&spProject->m_rwLock);
    for (const auto& [_, spResource] : spProject->m_spResourcesMap)
    {
      QReadLocker rLock(&spResource->m_rwLock);
      if (EResourceType::eDatabase == spResource->m_type._to_integral() &&
          QFileInfo(PhysicalResourcePath(spResource)).suffix() == joip_resource::c_sDialogueFileType)
      {
        vsResFiles.push_back(spResource);
      }
    }
  }

  m_vspDialoguesOnlyFlat.clear();
  m_spDataRootNode = dialogue_tree::LoadDialogues(vsResFiles);
  if (nullptr == m_spDataRootNode)
  {
    m_spDataRootNode = std::make_shared<CDialogueNode>();
  }

  ParseTreeItems(m_spDataRootNode, &m_vspDialoguesOnlyFlat);
}

//----------------------------------------------------------------------------------------
//
void CProjectDialogueManager::UnloadProject()
{
  m_vspDialoguesOnlyFlat.clear();
  m_spDataRootNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CDialogueNodeDialogue> CProjectDialogueManager::FindDialogue(const QString& sId)
{
  for (const auto& [sPathRef, nodeData] : m_vspDialoguesOnlyFlat)
  {
    if (sPathRef == sId)
    {
      return nodeData;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
std::vector<std::shared_ptr<CDialogueNodeDialogue>> CProjectDialogueManager::FindDialogue(const QRegularExpression& rx)
{
  std::vector<std::shared_ptr<CDialogueNodeDialogue>> vspOut;
  for (const auto& [sPath, nodeData]: m_vspDialoguesOnlyFlat)
  {
    QRegularExpressionMatch match = rx.match(sPath);
    if (match.hasMatch())
    {
      vspOut.push_back(nodeData);
    }
  }
  return vspOut;
}

//----------------------------------------------------------------------------------------
//
std::vector<std::shared_ptr<CDialogueNodeDialogue>> CProjectDialogueManager::FindDialogueByTag(const QStringList& vsTags)
{
  std::vector<std::shared_ptr<CDialogueNodeDialogue>> vspOut;
  for (const auto& [vsPath, nodeData]: m_vspDialoguesOnlyFlat)
  {
    bool bFoundAll = true;
    for (const QString& sTag : vsTags)
    {
      bFoundAll &= nodeData->m_tags.end() != nodeData->m_tags.find(sTag);
    }
    if (bFoundAll)
    {
      vspOut.push_back(nodeData);
    }
  }
  return vspOut;
}

//----------------------------------------------------------------------------------------
//
CProjectDialogueManagerWrapper::CProjectDialogueManagerWrapper(QPointer<QJSEngine> pEngine,
                                                           std::weak_ptr<CProjectDialogueManager> wpInstance) :
  m_wpInstance(wpInstance),
  m_pEngine(pEngine)
{
}
CProjectDialogueManagerWrapper::~CProjectDialogueManagerWrapper()
{
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogueManagerWrapper::dialogue(const QString& sId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto spDial = spInst->FindDialogue(sId);
    if (nullptr != spDial)
    {
      CDialogueWrapper* pWrapper =
          new CDialogueWrapper(m_pEngine, spDial);
      return m_pEngine->newQObject(pWrapper);
    }
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogueManagerWrapper::dialogueFromRx(const QString& sId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto vspDial = spInst->FindDialogue(QRegularExpression(sId));
    QJSValue val = m_pEngine->newArray(static_cast<qint32>(vspDial.size()));
    int iIndex = 0;
    for (const auto& spDial : vspDial)
    {
      CDialogueWrapper* pWrapper =
          new CDialogueWrapper(m_pEngine, spDial);
      val.setProperty(iIndex++, m_pEngine->newQObject(pWrapper));
    }
    return val;
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogueManagerWrapper::dialogueFromTags(const QStringList& vsId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto vspDial = spInst->FindDialogueByTag(vsId);
    QJSValue val = m_pEngine->newArray(static_cast<qint32>(vspDial.size()));
    int iIndex = 0;
    for (const auto& spDial : vspDial)
    {
      CDialogueWrapper* pWrapper =
          new CDialogueWrapper(m_pEngine, spDial);
      val.setProperty(iIndex++, m_pEngine->newQObject(pWrapper));
    }
    return val;
  }
  return QJSValue();
}
