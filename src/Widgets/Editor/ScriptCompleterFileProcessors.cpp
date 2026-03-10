#include "ScriptCompleterFileProcessors.h"

#include "Systems/Script/ScriptBackground.h"
#include "Systems/Script/ScriptDbWrappers.h"
#include "Systems/Script/ScriptDeviceController.h"
#include "Systems/Script/ScriptEval.h"
#include "Systems/Script/ScriptEventSender.h"
#include "Systems/Script/ScriptIcon.h"
#include "Systems/Script/ScriptMediaPlayer.h"
#include "Systems/Script/ScriptMetronome.h"
#include "Systems/Script/ScriptNotification.h"
#include "Systems/Script/ScriptSceneManager.h"
#include "Systems/Script/ScriptStorage.h"
#include "Systems/Script/ScriptTextBox.h"
#include "Systems/Script/ScriptThread.h"
#include "Systems/Script/ScriptTimer.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>

namespace
{
  //--------------------------------------------------------------------------------------
  //
  bool IsEowCharDefault(QChar c)
  {
    static const QString sEowDefault("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-=");
    return sEowDefault.contains(c) || c.isSpace();
  }

  //--------------------------------------------------------------------------------------
  //
  bool StringIsNumber(const QString& sStr)
  {
    bool bIsLongLong = false;
    bool bIsDouble = false;
    sStr.toLongLong(&bIsLongLong);
    sStr.toDouble(&bIsDouble);
    return bIsLongLong || bIsDouble;
  }

  //--------------------------------------------------------------------------------------
  //
  void HandleLineGeneric(IScriptCompleterFileProcessor* pProcessor,
                         int destRole, const QString& sLine,
                         SCompleterModelData* pOutData)
  {
    QString sLastWord;
    for (QChar c : sLine)
    {
      if (pProcessor->IsEndOfWordChar(c))
      {
        QString sString = sLastWord.simplified();
        if (!sString.isEmpty() && !StringIsNumber(sString))
        {
          pOutData->m_vsAllData[destRole].insert(sLastWord);
          sLastWord.clear();
        }
      }
      else
      {
        sLastWord += c;
      }
    }

    QString sString = sLastWord.simplified();
    if (!sString.isEmpty() && !StringIsNumber(sString))
    {
      pOutData->m_vsAllData[destRole].insert(sLastWord);
      sLastWord.clear();
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void ProcessFileGeneric(IScriptCompleterFileProcessor* pProcessor,
                          int destRole, QTextStream& data,
                          SCompleterModelData* pOutData,
                          const std::set<QString>& vsInbuiltKeywords)
  {
    QString sLine;
    do {
      sLine = data.readLine();
      QString sLine = data.readLine();
      HandleLineGeneric(pProcessor, destRole, sLine, pOutData);
    } while (!sLine.isNull());
    pOutData->m_vsAllData[destRole].insert(vsInbuiltKeywords.begin(), vsInbuiltKeywords.end());
  }

  //--------------------------------------------------------------------------------------
  //
  void InsertMetaObjectMethods(std::set<QString>& vsInbuiltKeywords,
                               const QMetaObject& staticMetaObject)
  {
    for (int i = 0; staticMetaObject.methodCount() > i; ++i)
    {
      QMetaMethod meth = staticMetaObject.method(i);
      if (meth.isValid())
      {
        vsInbuiltKeywords.insert(QString::fromLocal8Bit(meth.name()));
      }
    }
    for (int i = 0; staticMetaObject.propertyCount() > i; ++i)
    {
      QMetaProperty prop = staticMetaObject.property(i);
      if (prop.isValid())
      {
        vsInbuiltKeywords.insert(QString::fromLocal8Bit(prop.name()));
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InsertInbuiltMethodsAndProps(std::set<QString>& vsInbuiltKeywords)
  {
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptBackground::staticMetaObject);
    vsInbuiltKeywords.insert("background");
    InsertMetaObjectMethods(vsInbuiltKeywords, CProjectScriptWrapper::staticMetaObject);
    InsertMetaObjectMethods(vsInbuiltKeywords, CResourceScriptWrapper::staticMetaObject);
    InsertMetaObjectMethods(vsInbuiltKeywords, CSceneScriptWrapper::staticMetaObject);
    vsInbuiltKeywords.insert("scene");
    InsertMetaObjectMethods(vsInbuiltKeywords, CKinkWrapper::staticMetaObject);
    InsertMetaObjectMethods(vsInbuiltKeywords, CTagWrapper::staticMetaObject);
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptDeviceController::staticMetaObject);
    vsInbuiltKeywords.insert("deviceController");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptIcon::staticMetaObject);
    vsInbuiltKeywords.insert("icon");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptMediaPlayer::staticMetaObject);
    vsInbuiltKeywords.insert("mediaPlayer");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptMetronome::staticMetaObject);
    vsInbuiltKeywords.insert("metronome");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptNotification::staticMetaObject);
    vsInbuiltKeywords.insert("notification");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptSceneManager::staticMetaObject);
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptTextBox::staticMetaObject);
    vsInbuiltKeywords.insert("textBox");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptThread::staticMetaObject);
    vsInbuiltKeywords.insert("thread");
    InsertMetaObjectMethods(vsInbuiltKeywords, CScriptTimer::staticMetaObject);
    vsInbuiltKeywords.insert("timer");
  }
}

//----------------------------------------------------------------------------------------
//
CScriptCompleterFileProcessorJs::CScriptCompleterFileProcessorJs() :
  IScriptCompleterFileProcessor()
{
  // load the xml from the syntax-highlighting library for keywords
  QFile f(":/org.kde.syntax-highlighting/syntax/javascript.xml");
  if (f.open(QIODevice::ReadOnly))
  {
    QDomDocument doc;
    doc.setContent(&f);
    QDomNodeList listNodes = doc.elementsByTagName("list");
    for(qint32 i = 0; listNodes.count() > i; ++i)
    {
      QDomNode node = listNodes.at(i);
      if (node.isElement())
      {
        auto attrs = node.attributes();
        if (attrs.contains("name"))
        {
          m_vsInbuiltKeywords.insert(attrs.namedItem("name").toElement().text());
        }
        QDomNodeList itemNodes = node.toElement().elementsByTagName("item");
        for(qint32 j = 0; itemNodes.count() > j; ++j)
        {
          QDomNode itemNode = itemNodes.at(j);
          if (itemNode.isElement())
          {
            m_vsInbuiltKeywords.insert(node.toElement().text());
          }
        }
      }
    }
  }
  else
  {
    qWarning() << QObject::tr("Could no open js syntax definition file");
  }

  InsertInbuiltMethodsAndProps(m_vsInbuiltKeywords);
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptEvalJs::staticMetaObject);
  m_vsInbuiltKeywords.insert("evalRunner");
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptEventSenderJs::staticMetaObject);
  m_vsInbuiltKeywords.insert("eventSender");
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptStorageJs::staticMetaObject);
  m_vsInbuiltKeywords.insert("localStorage");
}

CScriptCompleterFileProcessorJs::~CScriptCompleterFileProcessorJs() = default;

//----------------------------------------------------------------------------------------
//
bool CScriptCompleterFileProcessorJs::IsEndOfWordChar(QChar c) const
{
  return IsEowCharDefault(c);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorJs::ProcessFile(int destRole, QTextStream& data,
                                                  SCompleterModelData* pOutData)
{
  ProcessFileGeneric(this, destRole, data, pOutData, m_vsInbuiltKeywords);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorJs::ProcessLine(int destRole, const QString& sLine,
                                                  SCompleterModelData* pOutData)
{
  HandleLineGeneric(this, destRole, sLine, pOutData);
}

//----------------------------------------------------------------------------------------
//
CScriptCompleterFileProcessorLua::CScriptCompleterFileProcessorLua() :
    IScriptCompleterFileProcessor()
{
  // load the xml from the syntax-highlighting library for keywords
  QFile f(":/org.kde.syntax-highlighting/syntax/lua.xml");
  if (f.open(QIODevice::ReadOnly))
  {
    QDomDocument doc;
    doc.setContent(&f);
    QDomNodeList listNodes = doc.elementsByTagName("list");
    for(qint32 i = 0; listNodes.count() > i; ++i)
    {
      QDomNode node = listNodes.at(i);
      if (node.isElement())
      {
        auto attrs = node.attributes();
        if (attrs.contains("name"))
        {
          m_vsInbuiltKeywords.insert(attrs.namedItem("name").toElement().text());
        }
        QDomNodeList itemNodes = node.toElement().elementsByTagName("item");
        for(qint32 j = 0; itemNodes.count() > j; ++j)
        {
          QDomNode itemNode = itemNodes.at(j);
          if (itemNode.isElement())
          {
            QString s = node.toElement().text();
            QStringList vs = s.split(".");
            for (const QString& sVal : vs)
            {
              m_vsInbuiltKeywords.insert(sVal);
            }
          }
        }
      }
    }
  }
  else
  {
    qWarning() << QObject::tr("Could no open js syntax definition file");
  }

  InsertInbuiltMethodsAndProps(m_vsInbuiltKeywords);
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptEvalLua::staticMetaObject);
  m_vsInbuiltKeywords.insert("evalRunner");
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptEventSenderLua::staticMetaObject);
  m_vsInbuiltKeywords.insert("eventSender");
  InsertMetaObjectMethods(m_vsInbuiltKeywords, CScriptStorageLua::staticMetaObject);
  m_vsInbuiltKeywords.insert("localStorage");
}
CScriptCompleterFileProcessorLua::~CScriptCompleterFileProcessorLua() = default;

//----------------------------------------------------------------------------------------
//
bool CScriptCompleterFileProcessorLua::IsEndOfWordChar(QChar c) const
{
  return IsEowCharDefault(c);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorLua::ProcessFile(int destRole, QTextStream& data,
                                                   SCompleterModelData* pOutData)
{
  ProcessFileGeneric(this, destRole, data, pOutData, m_vsInbuiltKeywords);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorLua::ProcessLine(int destRole, const QString& sLine,
                                                   SCompleterModelData* pOutData)
{
  HandleLineGeneric(this, destRole, sLine, pOutData);
}

//----------------------------------------------------------------------------------------
//
CScriptCompleterFileProcessorQml::CScriptCompleterFileProcessorQml() :
    IScriptCompleterFileProcessor()
{
  std::function<void(QFile&)> fn = [this](QFile& f) {
    if (f.open(QIODevice::ReadOnly))
    {
      QDomDocument doc;
      doc.setContent(&f);
      QDomNodeList listNodes = doc.elementsByTagName("list");
      for(qint32 i = 0; listNodes.count() > i; ++i)
      {
        QDomNode node = listNodes.at(i);
        if (node.isElement())
        {
          auto attrs = node.attributes();
          if (attrs.contains("name"))
          {
            m_vsInbuiltKeywords.insert(attrs.namedItem("name").toElement().text());
          }
          QDomNodeList itemNodes = node.toElement().elementsByTagName("item");
          for(qint32 j = 0; itemNodes.count() > j; ++j)
          {
            QDomNode itemNode = itemNodes.at(j);
            if (itemNode.isElement())
            {
              m_vsInbuiltKeywords.insert(node.toElement().text());
            }
          }
        }
      }
    }
    else
    {
      qWarning() << QObject::tr("Could no open js syntax definition file");
    }
  };

  // load the xml from the syntax-highlighting library for keywords
  QFile fQml(":/org.kde.syntax-highlighting/syntax/qml.xml");
  fn(fQml);
  QFile fJs(":/org.kde.syntax-highlighting/syntax/javascript.xml");
  fn(fJs);
}
CScriptCompleterFileProcessorQml::~CScriptCompleterFileProcessorQml() = default;

//----------------------------------------------------------------------------------------
//
bool CScriptCompleterFileProcessorQml::IsEndOfWordChar(QChar c) const
{
  return IsEowCharDefault(c);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorQml::ProcessFile(int destRole, QTextStream& data,
                                                   SCompleterModelData* pOutData)
{
  ProcessFileGeneric(this, destRole, data, pOutData, m_vsInbuiltKeywords);
}

//----------------------------------------------------------------------------------------
//
void CScriptCompleterFileProcessorQml::ProcessLine(int destRole, const QString& sLine,
                                                   SCompleterModelData* pOutData)
{
  HandleLineGeneric(this, destRole, sLine, pOutData);
}
