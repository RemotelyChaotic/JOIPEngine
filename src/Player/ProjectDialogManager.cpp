#include "ProjectDialogManager.h"

#include "Systems/Script/ScriptDbWrappers.h"

#include <QFileInfo>

CProjectDialogManager::CProjectDialogManager()
{

}

CProjectDialogManager::~CProjectDialogManager()
{

}

//----------------------------------------------------------------------------------------
//
namespace
{
  void ParseTreeItems(const std::shared_ptr<CDialogNode>& spNode,
                      std::vector<std::pair<QString, std::shared_ptr<CDialogNodeDialog>>>* pvspDataFlat)
  {
    for (const auto& spChildNode : spNode->m_vspChildren)
    {
      if (EDialogTreeNodeType::eDialog == spChildNode->m_type._to_integral())
      {
        pvspDataFlat->push_back({spChildNode->m_sName,
                                 std::dynamic_pointer_cast<CDialogNodeDialog>(spChildNode)});
      }
      ParseTreeItems(spChildNode, pvspDataFlat);
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CProjectDialogManager::LoadProject(const tspProject& spProject)
{
  std::vector<tspResource> vsResFiles;
  {
    QReadLocker lock(&spProject->m_rwLock);
    for (const auto& [_, spResource] : spProject->m_spResourcesMap)
    {
      QReadLocker rLock(&spResource->m_rwLock);
      if (EResourceType::eDatabase == spResource->m_type._to_integral() &&
          QFileInfo(PhysicalResourcePath(spResource)).suffix() == joip_resource::c_sDialogFileType)
      {
        vsResFiles.push_back(spResource);
      }
    }
  }

  m_vspDialogsOnlyFlat.clear();
  m_spDataRootNode = dialog_tree::LoadDialogs(vsResFiles);
  if (nullptr == m_spDataRootNode)
  {
    m_spDataRootNode = std::make_shared<CDialogNode>();
  }

  ParseTreeItems(m_spDataRootNode, &m_vspDialogsOnlyFlat);
}

//----------------------------------------------------------------------------------------
//
void CProjectDialogManager::UnloadProject()
{
  m_vspDialogsOnlyFlat.clear();
  m_spDataRootNode = nullptr;
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CDialogNodeDialog> CProjectDialogManager::FindDialog(const QString& sId)
{
  for (const auto& [sPathRef, nodeData] : m_vspDialogsOnlyFlat)
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
std::vector<std::shared_ptr<CDialogNodeDialog>> CProjectDialogManager::FindDialog(const QRegularExpression& rx)
{
  std::vector<std::shared_ptr<CDialogNodeDialog>> vspOut;
  for (const auto& [sPath, nodeData]: m_vspDialogsOnlyFlat)
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
std::vector<std::shared_ptr<CDialogNodeDialog>> CProjectDialogManager::FindDialogByTag(const QStringList& vsTags)
{
  std::vector<std::shared_ptr<CDialogNodeDialog>> vspOut;
  for (const auto& [vsPath, nodeData]: m_vspDialogsOnlyFlat)
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
CProjectDialogManagerWrapper::CProjectDialogManagerWrapper(QPointer<QJSEngine> pEngine,
                                                           std::weak_ptr<CProjectDialogManager> wpInstance) :
  m_wpInstance(wpInstance),
  m_pEngine(pEngine)
{
}
CProjectDialogManagerWrapper::~CProjectDialogManagerWrapper()
{
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogManagerWrapper::dialog(const QString& sId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto spDial = spInst->FindDialog(sId);
    if (nullptr != spDial)
    {
      CDialogWrapper* pWrapper =
          new CDialogWrapper(m_pEngine, spDial);
      return m_pEngine->newQObject(pWrapper);
    }
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogManagerWrapper::dialogFromRx(const QString& sId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto vspDial = spInst->FindDialog(QRegularExpression(sId));
    QJSValue val = m_pEngine->newArray(static_cast<qint32>(vspDial.size()));
    int iIndex = 0;
    for (const auto& spDial : vspDial)
    {
      CDialogWrapper* pWrapper =
          new CDialogWrapper(m_pEngine, spDial);
      val.setProperty(iIndex++, m_pEngine->newQObject(pWrapper));
    }
    return val;
  }
  return QJSValue();
}

//----------------------------------------------------------------------------------------
//
QJSValue CProjectDialogManagerWrapper::dialogFromTags(const QStringList& vsId)
{
  if (auto spInst = m_wpInstance.lock())
  {
    auto vspDial = spInst->FindDialogByTag(vsId);
    QJSValue val = m_pEngine->newArray(static_cast<qint32>(vspDial.size()));
    int iIndex = 0;
    for (const auto& spDial : vspDial)
    {
      CDialogWrapper* pWrapper =
          new CDialogWrapper(m_pEngine, spDial);
      val.setProperty(iIndex++, m_pEngine->newQObject(pWrapper));
    }
    return val;
  }
  return QJSValue();
}
