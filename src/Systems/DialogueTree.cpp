#include "DialogueTree.h"
#include "DatabaseManager.h"
#include "Settings.h""

#include "Database/Project.h"

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace
{
  const char c_sHasConditionNode[] = "bHasCondition";
  const char c_sFragmentsNode[] = "fragments";
  const char c_sTagsNode[] = "tags";

  const char c_sConditionNode[] = "sCondition";
  const char c_sStringNode[] = "sString";
  const char c_sSoundResourceNode[] = "sSoundResource";
  const char c_sWaitTimeMsNode[] = "iWaitTimeMs";
  const char c_sSkipableNode[] = "bSkipable";

  const char c_sFileNode[] = "sFileId";
  const char c_sNameNode[] = "sName";

  const char c_sUserAgent[] = "User-Agent";
}

CDialogueNode::CDialogueNode() :
  m_sFileId(),
  m_type(EDialogueTreeNodeType::eRoot),
  m_sName(dialogue_tree::c_sRootNodeName),
  m_bReadOnly(false)
{
}
CDialogueNode::~CDialogueNode() = default;

std::shared_ptr<CDialogueNode> CDialogueNode::Clone()
{
  std::shared_ptr<CDialogueNode> spRet = CloneImpl();
  spRet->m_sFileId = m_sFileId;
  spRet->m_type = m_type;
  spRet->m_sName = m_sName;
  spRet->m_bReadOnly = m_bReadOnly;
  return spRet;
}

void CDialogueNode::CopyFrom(const std::shared_ptr<CDialogueNode>& spNode)
{
  CopyFromImpl(spNode);
  m_sFileId = spNode->m_sFileId;
  m_type = spNode->m_type;
  m_sName = spNode->m_sName;
  m_bReadOnly = spNode->m_bReadOnly;
}

std::shared_ptr<CDialogueNode> CDialogueNode::CloneImpl()
{
  std::shared_ptr<CDialogueNode> spRet = std::make_shared<CDialogueNode>();
  spRet->m_sFileId = m_sFileId;
  spRet->m_type = m_type;
  spRet->m_sName = m_sName;
  spRet->m_bReadOnly = m_bReadOnly;
  return spRet;
}
void CDialogueNode::CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode)
{
  m_sFileId = spNode->m_sFileId;
  m_type = spNode->m_type;
  m_sName = spNode->m_sName;
  m_bReadOnly = spNode->m_bReadOnly;
}

//----------------------------------------------------------------------------------------
//
CDialogueNodeCategory::CDialogueNodeCategory() :
  CDialogueNode()
{
  m_type = EDialogueTreeNodeType::eCategory;
}
CDialogueNodeCategory::~CDialogueNodeCategory() = default;

std::shared_ptr<CDialogueNode> CDialogueNodeCategory::CloneImpl()
{
  std::shared_ptr<CDialogueNodeCategory> spRet = std::make_shared<CDialogueNodeCategory>();
  return spRet;
}
void CDialogueNodeCategory::CopyFromImpl(const std::shared_ptr<CDialogueNode>&)
{
}

//----------------------------------------------------------------------------------------
//
CDialogueNodeDialogue::CDialogueNodeDialogue() :
  CDialogueNode()
{
  m_type = EDialogueTreeNodeType::eDialogue;
}
CDialogueNodeDialogue::~CDialogueNodeDialogue() = default;

std::shared_ptr<CDialogueNode> CDialogueNodeDialogue::CloneImpl()
{
  std::shared_ptr<CDialogueNodeDialogue> spRet = std::make_shared<CDialogueNodeDialogue>();
  spRet->m_tags = m_tags;
  spRet->m_bHasCondition = m_bHasCondition;
  return spRet;
}
void CDialogueNodeDialogue::CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode)
{
  auto spImpl = std::dynamic_pointer_cast<CDialogueNodeDialogue>(spNode);
  m_tags = spImpl->m_tags;
  m_bHasCondition = spImpl->m_bHasCondition;
}

//----------------------------------------------------------------------------------------
//
CDialogueData::CDialogueData() :
  CDialogueNode()
{
  m_type = EDialogueTreeNodeType::eDialogueFragment;
}
CDialogueData::~CDialogueData() = default;

std::shared_ptr<CDialogueNode> CDialogueData::CloneImpl()
{
  std::shared_ptr<CDialogueData> spRet = std::make_shared<CDialogueData>();
  spRet->m_sCondition = m_sCondition;
  spRet->m_sString = m_sString;
  spRet->m_sSoundResource = m_sSoundResource;
  spRet->m_iWaitTimeMs = m_iWaitTimeMs;
  spRet->m_bSkipable = m_bSkipable;
  return spRet;
}
void CDialogueData::CopyFromImpl(const std::shared_ptr<CDialogueNode>& spNode)
{
  auto spImpl = std::dynamic_pointer_cast<CDialogueData>(spNode);
  m_sCondition = spImpl->m_sCondition;
  m_sString = spImpl->m_sString;
  m_sSoundResource = spImpl->m_sSoundResource;
  m_iWaitTimeMs = spImpl->m_iWaitTimeMs;
  m_bSkipable = spImpl->m_bSkipable;
}


namespace
{
  //--------------------------------------------------------------------------------------
  //
  /* json-parse the people - with custom error handler */
  class CDialogueJsonErrorHandler : public nlohmann::json_schema::basic_error_handler
  {
  public:
    CDialogueJsonErrorHandler(QString* pError) : m_pError(pError)
    {
    }

    void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer,
               const nlohmann::json &instance,
               const std::string &message) override
    {
      nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
      if (nullptr != m_pError)
      {
        *m_pError =
            QString("Validation of schema failed:") + QString::fromStdString(pointer.to_string()) +
            ":" + QString::fromStdString(message);
      }
    }

    QString* m_pError;
  };

  //--------------------------------------------------------------------------------------
  //
  void RecursiveRead(const nlohmann::json& root,
                     const tspProject& spProj,
                     const QString& sFileId,
                     std::shared_ptr<CDialogueNode> spRoot,
                     bool bReadOnly)
  {
    for(auto it = root.begin(); it != root.end(); ++it)
    {
      if (it->is_object())
      {
        // dialogue node
        if (it->contains(c_sHasConditionNode))
        {
          auto itHasCondition = it->find(c_sHasConditionNode);
          auto itTags = it->find(c_sTagsNode);
          auto itFragments = it->find(c_sFragmentsNode);
          auto itDlgFile = it->find(c_sFileNode);
          if (it->end() != itHasCondition && it->end() != itTags && it->end() != itFragments &&
              itTags->is_array() && itFragments->is_array())
          {
            QString sKey = QString::fromStdString(it.key());
            sKey = dialogue_tree::EnsureUniqueName(sKey, spRoot, nullptr);

            auto spChild = std::make_shared<CDialogueNodeDialogue>();
            spRoot->m_vspChildren.push_back(spChild);
            spChild->m_wpParent = spRoot;
            spChild->m_bReadOnly = bReadOnly;
            spChild->m_sName = sKey;

            spChild->m_sFileId = sFileId;
            if (it->end() != itDlgFile && itDlgFile->is_string())
            {
              spChild->m_sFileId = QString::fromStdString(itDlgFile.value().get<std::string>());
            }

            spChild->m_bHasCondition = itHasCondition.value().get<bool>();

            QReadLocker locker(&spProj->m_rwLock);
            for (const auto& tag : itTags.value())
            {
              if (tag.is_string())
              {
                const QString sTag = QString::fromStdString(tag.get<std::string>());
                auto itTag = spProj->m_baseData.m_vspTags.find(sTag);
                if (spProj->m_baseData.m_vspTags.end() != itTag)
                {
                  spChild->m_tags.insert({sTag, itTag->second});
                }
              }
            }
            RecursiveRead(itFragments.value(), spProj, sFileId, spChild, bReadOnly);
          }
        }
        // dialogue fragment
        else if (it->contains(c_sStringNode))
        {
          auto itName = it->find(c_sNameNode);
          auto itCond = it->find(c_sConditionNode);
          auto itString = it->find(c_sStringNode);
          auto itSoundRes = it->find(c_sSoundResourceNode);
          auto itWaitTime = it->find(c_sWaitTimeMsNode);
          auto itSkippable = it->find(c_sSkipableNode);
          auto itFileId = it->find(c_sFileNode);
          if (it->end() != itCond && it->end() != itString &&
              it->end() != itSoundRes && it->end() != itWaitTime &&
              it->end() != itSkippable && it->end() != itName &&
              itCond->is_string() && itString->is_string() && itSoundRes->is_string() &&
              itWaitTime->is_number_integer() && itSkippable->is_boolean())
          {
            QString sKey = QString::fromStdString(itName.value().get<std::string>());
            sKey = dialogue_tree::EnsureUniqueName(sKey, spRoot, nullptr);

            auto spFrag = std::make_shared<CDialogueData>();
            spRoot->m_vspChildren.push_back(spFrag);
            spFrag->m_wpParent = spRoot;
            spFrag->m_bReadOnly = bReadOnly;

            spFrag->m_sName = sKey;
            spFrag->m_sCondition = QString::fromStdString(itCond.value().get<std::string>());
            spFrag->m_sString = QString::fromStdString(itString.value().get<std::string>());
            spFrag->m_sSoundResource = QString::fromStdString(itSoundRes.value().get<std::string>());
            spFrag->m_iWaitTimeMs = itWaitTime.value().get<int>();
            spFrag->m_bSkipable = itSkippable.value().get<bool>();

            spFrag->m_sFileId = sFileId;
            if (it->end() != itFileId && itFileId->is_string())
            {
              spFrag->m_sFileId = QString::fromStdString(itFileId.value().get<std::string>());
            }
          }
        }
        // category node
        else
        {
          QString sKey = QString::fromStdString(it.key());
          sKey = dialogue_tree::EnsureUniqueName(sKey, spRoot, nullptr);
          std::shared_ptr<CDialogueNodeCategory> spChild = nullptr;
          auto itFound = std::find_if(spRoot->m_vspChildren.begin(), spRoot->m_vspChildren.end(),
                                 [sKey](const std::shared_ptr<CDialogueNode>& spChild) {
              return sKey == spChild->m_sName &&
                   EDialogueTreeNodeType::eCategory == spChild->m_type._to_integral();
          });
          if (spRoot->m_vspChildren.end() == itFound)
          {
            spChild = std::make_shared<CDialogueNodeCategory>();
            spChild->m_sName = sKey;
            spChild->m_wpParent = spRoot;
            spChild->m_bReadOnly = bReadOnly;
            spRoot->m_vspChildren.push_back(spChild);
          }
          else
          {
            spChild = std::dynamic_pointer_cast<CDialogueNodeCategory>(*itFound);
            if (nullptr == spChild)
            {
              spChild = std::make_shared<CDialogueNodeCategory>();
              spChild->m_sName = sKey;
              spChild->m_wpParent = spRoot;
              spChild->m_bReadOnly = bReadOnly;
              spRoot->m_vspChildren.push_back(spChild);
            }
          }
          spChild->m_bReadOnly &= bReadOnly;

          RecursiveRead(it.value(), spProj, sFileId, spChild, bReadOnly);
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool ParseDialogueFile(const QByteArray& arr,
                         std::shared_ptr<CDialogueNode>& spRoot,
                         bool bReadOnly, const tspProject& spProj,
                         const QString& sFileId)
  {
    static QString c_sSchema = []() -> QString {
      QFile f(":/resources/data/dialog_schema.json");
      if (f.open(QIODevice::ReadOnly))
      {
        return QString::fromUtf8(f.readAll());
      }
      qWarning() << QObject::tr("Could not open inbuilt dialogue json.");
      return QString();
    }();

    QString sError;

    CDialogueJsonErrorHandler errorHandler(&sError);
    nlohmann::json_schema::json_validator validator;
    nlohmann::json dialogue;

    try
    {
      dialogue = nlohmann::json::parse(QString::fromUtf8(arr).toStdString());
    }
    catch (const std::exception &e)
    {
      qWarning() << QObject::tr("Loading json failed: %1").arg(e.what());
      return false;
    }

    nlohmann::json schema;
    try
    {
      schema = nlohmann::json::parse(c_sSchema.toStdString());
    }
    catch (const std::exception &e)
    {
      qWarning() << QObject::tr("Loading of schema failed: %1").arg(e.what());
      return false;
    }

    try
    {
      validator.set_root_schema(schema);
    }
    catch (const std::exception &e)
    {
      qWarning() << QObject::tr("Validation of schema failed: %1").arg(e.what());
      return false;
    }

    bool bOk = true;
    validator.validate(dialogue, errorHandler);
    if (static_cast<bool>(errorHandler))
    {
      bOk = false;
    }

    RecursiveRead(dialogue, spProj, sFileId, spRoot, bReadOnly);

    return bOk;
  }

  //--------------------------------------------------------------------------------------
  //
  bool ReadFialogFile(std::shared_ptr<CDialogueNode>& spRoot,
                      const QString& sPath, const QString& sFileId, bool bReadOnly,
                      const tspProject& spProj)
  {
    QFile f(sPath);
    if (f.exists() && f.open(QIODevice::ReadOnly))
    {
      QByteArray arr = f.readAll();
      return ParseDialogueFile(arr, spRoot, bReadOnly, spProj, sFileId);
    }

    return false;
  }

  //--------------------------------------------------------------------------------------
  //
  bool FetchDialogueFile(std::shared_ptr<CDialogueNode>& spRoot,
                         const SResourcePath& sPath, const QString& sFileId,
                         const tspProject& spProj)
  {
    QByteArray arr;
    QEventLoop loop;
    std::shared_ptr<QNetworkAccessManager> spManager = std::make_shared<QNetworkAccessManager>();
    QNetworkRequest req(static_cast<QUrl>(sPath));
    req.setRawHeader(c_sUserAgent, CSettings::c_sApplicationName.toUtf8());
    QPointer<QNetworkReply> pReply = spManager->get(req);
    QObject::connect(pReply, &QNetworkReply::finished,
            &loop, [pReply, &loop, &arr](){
              if(nullptr != pReply)
              {
                arr = pReply->readAll();
                bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                assert(bOk); Q_UNUSED(bOk)
              }
              else
              {
                qCritical() << "QNetworkReply object was destroyed too early.";
              }
            });
    QObject::connect(pReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            &loop, [pReply, &loop](QNetworkReply::NetworkError error){
              Q_UNUSED(error)
              if (nullptr != pReply)
              {
                qWarning() << QObject::tr(QT_TR_NOOP("Error fetching remote resource: %1"))
                                  .arg(pReply->errorString());
                bool bOk = QMetaObject::invokeMethod(&loop, "quit", Qt::QueuedConnection);
                assert(bOk); Q_UNUSED(bOk)
              }
              else
              {
                qCritical() << "QNetworkReply object was destroyed too early.";
              }
            });

    loop.exec();

    if (nullptr != pReply) { delete pReply; }

    return ParseDialogueFile(arr, spRoot, true, spProj, sFileId);
  }

  //--------------------------------------------------------------------------------------
  //
  using tSeparatedTree = std::map<tspResource, std::vector<std::shared_ptr<CDialogueNode>>>;
  tSeparatedTree SeparateTree(const std::shared_ptr<CDialogueNode>& spDialogueNodeTree,
                              const tspProject& spProject)
  {
    tSeparatedTree out;
    for (const auto& spNode : spDialogueNodeTree->m_vspChildren)
    {
      if (EDialogueTreeNodeType::eDialogue == spNode->m_type._to_integral() ||
          EDialogueTreeNodeType::eCategory == spNode->m_type._to_integral())
      {
        tSeparatedTree temp = SeparateTree(spNode, spProject);

        for (auto& [spRes, vspFileChildren] : temp)
        {
          std::shared_ptr<CDialogueNode> spNodeCopy = nullptr;
          auto it = out.find(spRes);
          if (out.end() == it)
          {
            spNodeCopy = spNode->Clone();
            out[spRes].push_back(spNodeCopy);
          }
          else
          {
            auto itFound = std::find_if(it->second.begin(), it->second.end(),
                                      [&spNode](const std::shared_ptr<CDialogueNode>& spCheck) {
              return spNode->m_sName == spCheck->m_sName;
            });
            if (it->second.end() != itFound)
            {
              spNodeCopy = *itFound;
            }
            else
            {
              spNodeCopy = spNode->Clone();
              out[spRes].push_back(spNodeCopy);
            }
          }
          for (auto& spNodeChild : vspFileChildren)
          {
            spNodeChild->m_wpParent = spNodeCopy;
            spNodeCopy->m_vspChildren.push_back(spNodeChild);
          }
        }
      }
      else if (EDialogueTreeNodeType::eDialogueFragment == spNode->m_type._to_integral())
      {
        std::shared_ptr<CDialogueNode> spNodeCopy = spNode->Clone();
        auto it = spProject->m_baseData.m_spResourcesMap.find(spNodeCopy->m_sFileId);
        if (spProject->m_baseData.m_spResourcesMap.end() != it)
        {
          out[it->second].push_back(spNodeCopy);
        }
      }
      else
      {
        continue;
      }
    }
    return out;
  }

  //--------------------------------------------------------------------------------------
  //
  void WriteNodeChildren(QJsonObject& parent,
                         const std::vector<std::shared_ptr<CDialogueNode>>& vspRootNodes,
                         bool bWriteFileId)
  {
    for (const auto& spNode : vspRootNodes)
    {
      switch (spNode->m_type)
      {
        case EDialogueTreeNodeType::eCategory:
        {
          QJsonObject obj;
          WriteNodeChildren(obj, spNode->m_vspChildren, bWriteFileId);
          parent.insert(spNode->m_sName, obj);
        } break;
        case EDialogueTreeNodeType::eDialogue:
        {
          auto spDialogue = std::static_pointer_cast<CDialogueNodeDialogue>(spNode);
          QJsonObject obj;
          QJsonArray arrTags;
          QJsonArray arrFragments;
          for (const auto& [sName, _] : spDialogue->m_tags)
          {
            arrTags.push_back(sName);
          }
          QJsonObject arrFragmentsO;
          WriteNodeChildren(arrFragmentsO, spNode->m_vspChildren, bWriteFileId);
          for (qint32 i = 0; arrFragmentsO.count() > i; ++i)
          {
            arrFragments.push_back((arrFragmentsO.begin()+i).value());
          }
          obj.insert(c_sHasConditionNode, spDialogue->m_bHasCondition);
          obj.insert(c_sTagsNode, arrTags);
          obj.insert(c_sFragmentsNode, arrFragments);
          if (bWriteFileId)
          {
            obj.insert(c_sFileNode, spDialogue->m_sFileId);
          }
          parent.insert(spNode->m_sName, obj);
        } break;
        case EDialogueTreeNodeType::eDialogueFragment:
        {
          auto spDialogue = std::static_pointer_cast<CDialogueData>(spNode);
          QJsonObject obj;
          obj.insert(c_sNameNode, spDialogue->m_sName);
          obj.insert(c_sConditionNode, spDialogue->m_sCondition);
          obj.insert(c_sStringNode, spDialogue->m_sString);
          obj.insert(c_sSoundResourceNode, spDialogue->m_sSoundResource);
          obj.insert(c_sWaitTimeMsNode, spDialogue->m_iWaitTimeMs);
          obj.insert(c_sSkipableNode, spDialogue->m_bSkipable);
          if (bWriteFileId)
          {
            obj.insert(c_sFileNode, spDialogue->m_sFileId);
          }
          parent.insert(spNode->m_sName, obj);
        } break;
        case EDialogueTreeNodeType::eRoot: break;
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool WriteSingleFile(const tspResource& spResource,
                       const std::vector<std::shared_ptr<CDialogueNode>>& vspRootNodes)
  {
    QJsonObject root;
    WriteNodeChildren(root, vspRootNodes, false);
    QJsonDocument doc(root);

    QReadLocker locker(&spResource->m_rwLock);
    QFile f(spResource->PhysicalResourcePath());
    if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
      return false;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    return true;
  }
}

//----------------------------------------------------------------------------------------
//
namespace
{
  void IterateNodeChildrenAdgGetName(QStringList* pvsList,
                                     const std::shared_ptr<CDialogueNode>& spRoot,
                                     const std::shared_ptr<CDialogueNode>& spExcept)
  {
    for (const auto& spChild : spRoot->m_vspChildren)
    {
      if (spExcept != spChild)
      {
        (*pvsList) << spChild->m_sName;
      }
      IterateNodeChildrenAdgGetName(pvsList, spChild, spExcept);
    }
  }
}

namespace dialogue_tree
{
  //--------------------------------------------------------------------------------------
  //
  QString EnsureUniqueName(const QString& sStr,
                           const std::shared_ptr<CDialogueNode>& spParent,
                           const std::shared_ptr<CDialogueNode>& spExcept)
  {
    QString sOut = sStr;
    QStringList vsForbidden;

    // get root
    auto spRoot = spParent;
    auto spRootTest = spParent;
    while (nullptr != spRootTest)
    {
      spRootTest = spRoot->m_wpParent.lock();
      if (nullptr != spRootTest)
      {
        spRoot = spRootTest;
      }
    }

    IterateNodeChildrenAdgGetName(&vsForbidden, spRoot, spExcept);

    // insert a counter
    qint32 iNrIterations = 1;
    qint32 iFound = vsForbidden.indexOf(sOut);
    while (-1 != iFound)
    {
      qint32 iCounter = vsForbidden[iFound].lastIndexOf(QRegExp("_\\d$"));
      if (-1 != iCounter)
      {
        sOut = vsForbidden[iFound].replace(iCounter, vsForbidden[iFound].length()-iCounter,
                                           "_" + QString::number(iNrIterations++));
      }
      else
      {
        sOut = vsForbidden[iFound] + "_1";
      }
      iFound = vsForbidden.indexOf(sOut);
    }

    return sOut;
  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CDialogueNode> DeserializeNode(const QByteArray& arr, const tspProject& spProject)
  {
    nlohmann::json dialogue;

    try
    {
      dialogue = nlohmann::json::parse(QString::fromUtf8(arr).toStdString());
    }
    catch (const std::exception &e)
    {
      qWarning() << QObject::tr("Loading json failed: %1").arg(e.what());
      return nullptr;
    }

    std::shared_ptr<CDialogueNode> spRoot = std::make_shared<CDialogueNode>();
    RecursiveRead(dialogue, spProject, QString(), spRoot, false);

    if (spRoot->m_vspChildren.size() > 0)
    {
      auto spChild = spRoot->m_vspChildren[0];
      spChild->m_wpParent.reset();
      return spChild;
    }
    return nullptr;
  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CDialogueNode> LoadDialogues(const std::vector<tspResource>& vsFiles, bool* bpErrors)
  {
    if (nullptr != bpErrors) { *bpErrors = true; }
    std::shared_ptr<CDialogueNode> spRoot = std::make_shared<CDialogueNode>();
    for (const tspResource& spResource : vsFiles)
    {
      QReadLocker locker(&spResource->m_rwLock);
      if (EResourceType::eDatabase != spResource->m_type._to_integral()) { continue; }
      if (spResource->m_sPath.Suffix() != joip_resource::c_sDialogueFileType)
      { continue; }

      if (spResource->m_sPath.IsLocalFile())
      {
        CDatabaseManager::LoadBundle(spResource->m_spParent, spResource->m_sResourceBundle);
        QString sPath = spResource->ResourceToAbsolutePath();
        bool bOk = ReadFialogFile(spRoot,
                                  sPath,
                                  spResource->m_sName,
                                  !spResource->m_sResourceBundle.isEmpty(),
                                  spResource->m_spParent);
        if (nullptr != bpErrors) { *bpErrors &= bOk; }
      }
      else
      {
        bool bOk = FetchDialogueFile(spRoot,
                                     spResource->m_sPath,
                                     spResource->m_sName,
                                     spResource->m_spParent);
        if (nullptr != bpErrors) { *bpErrors &= bOk; }
      }
    }

    return spRoot;
  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CDialogueNode> LoadDialoguesFromSource(const std::vector<QUrl>& vsFiles,
                                                         const tspProject& spProject,
                                                         bool* bpErrors)
  {
    if (nullptr != bpErrors) { *bpErrors = true; }
    std::shared_ptr<CDialogueNode> spRoot = std::make_shared<CDialogueNode>();
    for (const QUrl& sPathUrl : vsFiles)
    {
      if (SResourcePath::IsLocalFileP(sPathUrl))
      {
        QString sPath = sPathUrl.toLocalFile();
        bool bOk = ReadFialogFile(spRoot,
                                  sPath,
                                  QString(),
                                  true,
                                  spProject);
        if (nullptr != bpErrors) { *bpErrors &= bOk; }
      }
      else
      {
        bool bOk = FetchDialogueFile(spRoot,
                                     sPathUrl,
                                     QString(),
                                     spProject);
        if (nullptr != bpErrors) { *bpErrors &= bOk; }
      }
    }

    return spRoot;
  }

  //--------------------------------------------------------------------------------------
  //
  void SaveDialogues(const std::shared_ptr<CDialogueNode>& spDialogueNodeTree, const tspProject& spProject)
  {
    QReadLocker locker(&spProject->m_rwLock);
    tSeparatedTree vSeparatedTree = SeparateTree(spDialogueNodeTree, spProject);

    for (const auto& [spResource, vspNodeRoot] : vSeparatedTree)
    {
      if (vspNodeRoot.empty()) { continue; }
      if (vspNodeRoot.front()->m_bReadOnly) { continue; }
      bool bOk = WriteSingleFile(spResource, vspNodeRoot);
      if (!bOk)
      {
        QReadLocker l(&spResource->m_rwLock);
        qWarning() << QObject::tr("Error writing dialogue file: %1.").arg(spResource->m_sName);
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  QByteArray SerializeNode(const std::shared_ptr<CDialogueNode>& spNode)
  {
    QJsonObject root;
    WriteNodeChildren(root, {spNode}, true);
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
  }
}
