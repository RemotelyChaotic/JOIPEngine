#include "ScriptTextBox.h"
#include "Application.h"
#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"

#include "Systems/EOS/CommandEosChoiceBase.h"
#include "Systems/EOS/CommandEosPromptBase.h"
#include "Systems/EOS/CommandEosSayBase.h"
#include "Systems/EOS/EosCommands.h"
#include "Systems/EOS/EosHelpers.h"

#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"

#include "Systems/Sequence/SequenceTextBoxRunner.h"

#include "Systems/Project.h"
#include "Systems/Resource.h"

#include <QDateTime>
#include <QDebug>
#include <QEventLoop>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTimer>
#include <QUuid>

#include <limits>

namespace
{
  const qint32 c_iDelayBaseMs = 1'500;
  const qint32 c_iDelayPerCharMs = 30;
  const qint32 c_iDelayPerCharMaxMs = 8'000;
  const qint32 c_iDelayPerWordMs = 300;

  qint64 EstimateDurationBasedOnText(const QString& sText)
  {
    QString sPlainText = QTextDocumentFragment::fromHtml(sText).toPlainText();
     return (
       c_iDelayBaseMs +
       std::max(
         std::min(sPlainText.size() * c_iDelayPerCharMs, c_iDelayPerCharMaxMs),
         (sPlainText.count(QRegExp("\\s")) + 1) * c_iDelayPerWordMs
       )
     );
  }
}

//----------------------------------------------------------------------------------------
//
CTextBoxSignalEmitter::CTextBoxSignalEmitter() :
  CScriptRunnerSignalEmiter()
{

}
CTextBoxSignalEmitter::~CTextBoxSignalEmitter()
{

}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptObjectBase> CTextBoxSignalEmitter::CreateNewScriptObject(QPointer<QJSEngine> pEngine)
{
  return std::make_shared<CScriptTextBox>(this, pEngine);
}
std::shared_ptr<CScriptObjectBase> CTextBoxSignalEmitter::CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser)
{
  return std::make_shared<CEosScriptTextBox>(this, pParser);
}
std::shared_ptr<CScriptObjectBase> CTextBoxSignalEmitter::CreateNewScriptObject(QtLua::State* pState)
{
  return std::make_shared<CScriptTextBox>(this, pState);
}
std::shared_ptr<CScriptObjectBase> CTextBoxSignalEmitter::CreateNewSequenceObject()
{
  return std::make_shared<CSequenceTextBoxRunner>(this);
}

//----------------------------------------------------------------------------------------
//
CScriptTextBox::CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}
CScriptTextBox::CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QtLua::State* pState)  :
  CJsScriptObjectBase(pEmitter, pState),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptTextBox::~CScriptTextBox()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setBackgroundColors(QVariant colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setBackgroundColors");
  emit SignalEmitter<CTextBoxSignalEmitter>()->textBackgroundColorsChanged(colorsConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextAlignment(qint32 alignment)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTextBoxSignalEmitter>()->textAlignmentChanged(alignment);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextColors(QVariant colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setTextColors");
  emit SignalEmitter<CTextBoxSignalEmitter>()->textColorsChanged(colorsConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextPortrait(QVariant resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  if (nullptr != spSignalEmitter)
  {
    QString sError;
    std::optional<QString> optRes =
        script::ParseResourceFromScriptVariant(resource, m_wpDbManager.lock(),
                                               m_spProject,
                                               "setTextPortrait", &sError);
    if (optRes.has_value())
    {
      QString resRet = optRes.value();
      emit spSignalEmitter->textPortraitChanged(resRet);
    }
    else
    {
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptTextBox::showButtonPrompts(QVariant vsLabels)
{
  return showButtonPrompts(vsLabels, QString());
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptTextBox::showButtonPrompts(QVariant vsLabels, QString sStoreInto)
{
  if (!CheckIfScriptCanRun()) { return -1; }
  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  if (nullptr == pSignalEmitter) { return -1; }

  QString sRequestId = QUuid::createUuid().toString();
  QString sError;
  auto optvsStringLabels =
      script::ParseStringListFromScriptVariant(vsLabels, "showButtonPrompts", &sError);
  if (!optvsStringLabels.has_value())
  {
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    return -1;
  }

  emit pSignalEmitter->showButtonPrompts(optvsStringLabels.value(), sStoreInto, sRequestId,
                                         true);

  // local loop to wait for answer
  qint32 iReturnValue = -1;
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::showButtonReturnValue,
            this, [this, &iReturnValue, sRequestId](qint32 iIndexSelected, QString sRequestIdRet)
  {
    if (sRequestId == sRequestIdRet)
    {
      iReturnValue = iIndexSelected;
      emit this->SignalQuitLoop();
    }
  }, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return iReturnValue;
}

//----------------------------------------------------------------------------------------
//
QVariant CScriptTextBox::getDialog(QVariant data)
{
  if (!CheckIfScriptCanRun()) { return QVariant(); }

  QString sStringRet;
  qint32 iTimeRet = -1;
  bool bWaitRet = false;
  QString sSoundResource;
  QStringList vsTagsRet;

  bool bOk = GetDialogData(data, sStringRet, iTimeRet, bWaitRet, sSoundResource, vsTagsRet);

  if (bOk)
  {
    QVariantMap varMap;
    varMap.insert("string", sStringRet);
    varMap.insert("waitTimeMs", iTimeRet);
    varMap.insert("skipable", bWaitRet);
    varMap.insert("soundResource", sSoundResource);
    varMap.insert("tags", vsTagsRet);
    return varMap;
  }

  return QVariant();
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showDialog(QVariant data)
{
  if (!CheckIfScriptCanRun()) { return; }

  QString sStringRet;
  qint32 iTimeRet = -1;
  bool bWaitRet = false;
  QString sSoundResource;
  QStringList vsTagsRet;

  bool bOk = GetDialogData(data, sStringRet, iTimeRet, bWaitRet, sSoundResource, vsTagsRet);

  if (bOk)
  {
    showText(sStringRet, iTimeRet, bWaitRet);
  }
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText)
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CTextBoxSignalEmitter>()->showText(sText, -1, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText, double dWaitTime)
{
  if (!CheckIfScriptCanRun()) { return; }
  showText(sText, dWaitTime, false);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText, double dWaitTime, bool bSkipable)
{
  if (!CheckIfScriptCanRun()) { return; }
  showText(sText, dWaitTime, bSkipable, QString());
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText, double dWaitTime, bool bSkipable, QString sResource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();

  if (0 > dWaitTime)
  {
    dWaitTime = static_cast<double>(EstimateDurationBasedOnText(sText)) / 1000.0;
  }

  if (0 < dWaitTime)
  {
    QDateTime lastTime = QDateTime::currentDateTime();
    qint32 iTimeLeft = dWaitTime * 1000;

    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(20);
    QEventLoop loop;
    QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    QMetaObject::Connection pauseLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::pauseExecution, &loop, [&timer]() {
        timer.stop();
      }, Qt::QueuedConnection);
    QMetaObject::Connection resumeLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::resumeExecution, &loop, [&timer]() {
        timer.start();
      }, Qt::QueuedConnection);

    QMetaObject::Connection timeoutLoop =
      connect(&timer, &QTimer::timeout, &loop, [&loop, &iTimeLeft, &lastTime]() {
        QDateTime newTime = QDateTime::currentDateTime();
        iTimeLeft -= newTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch();
        lastTime = newTime;
        if (0 >= iTimeLeft)
        {
          emit loop.exit();
        }
      });

    QMetaObject::Connection skipLoop;
    if (bSkipable)
    {
      skipLoop =
        connect(pSignalEmitter, &CTextBoxSignalEmitter::waitSkipped,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }

    QTimer::singleShot(0, this, [&pSignalEmitter,sText,bSkipable,dWaitTime,sResource]() {
      emit pSignalEmitter->showText(sText, bSkipable ? dWaitTime : 0, sResource);
    });

    timer.start();
    loop.exec();
    timer.stop();
    timer.disconnect();
    loop.disconnect();

    disconnect(interruptLoop);
    disconnect(interruptThisLoop);
    disconnect(pauseLoop);
    disconnect(resumeLoop);
    disconnect(timeoutLoop);
    if (bSkipable)
    {
      disconnect(skipLoop);
    }
  }
}

//----------------------------------------------------------------------------------------
//
QString CScriptTextBox::showInput()
{
  return showInput(QString());
}

//----------------------------------------------------------------------------------------
//
QString CScriptTextBox::showInput(QString sStoreInto)
{
  if (!CheckIfScriptCanRun()) { return QString(); }

  QString sRequestId = QUuid::createUuid().toString();
  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter, sStoreInto, sRequestId]() {
    emit pSignalEmitter->showInput(sStoreInto, sRequestId, false);
  });

  // local loop to wait for answer
  QString sReturnValue = QString();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::showInputReturnValue,
            this, [this, &sReturnValue, sRequestId](QString sInput, QString sRequestIdRet)
  {
    if (sRequestId == sRequestIdRet)
    {
      sReturnValue = sInput;
      sReturnValue.detach(); // fixes some crashes with QJSEngine
      emit this->SignalQuitLoop();
    }
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return sReturnValue;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::clear()
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CTextBoxSignalEmitter>()->clearText();
}

//----------------------------------------------------------------------------------------
//
std::vector<QColor> CScriptTextBox::GetColors(const QVariant& colors, const QString& sSource)
{
  QString sError;
  std::optional<std::vector<QColor>> colorsRetOpt =
      script::ParseColorsFromScriptVariant(colors, 255, sSource, &sError);
  if (colorsRetOpt.has_value())
  {
    return colorsRetOpt.value();
  }
  else
  {
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    return {};
  }
}

//----------------------------------------------------------------------------------------
//
bool CScriptTextBox::GetDialogData(QVariant data, QString& sStringRet, qint32& iTimeRet, bool& bWaitRet,
                                   QString& sSoundResource, QStringList& vsTagsRet)
{
  QString sId = QUuid::createUuid().toString();
  QString sString = QString();
  QStringList vsTags;

  if (data.type() == QVariant::String)
  {
    sString = data.toString();
  }

  if (sString.isEmpty())
  {
    QString sError;
    auto optvsStringLabels =
        script::ParseTagListFromScriptVariant(data, m_wpDbManager.lock(),
                                              m_spProject, "showDialog", &sError);
    if (!optvsStringLabels.has_value())
    {
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      return false;
    }
    vsTags = *optvsStringLabels;
  }

  return GetDialogFromUi(sId, sString, vsTags, sStringRet, iTimeRet, bWaitRet,
                         sSoundResource, vsTagsRet);
}

//----------------------------------------------------------------------------------------
//
bool CScriptTextBox::GetDialogFromUi(QString sId, QString sString, QStringList vsTags,
                                     QString& sStringRet, qint32& iTimeRet, bool& bWaitRet,
                                     QString& sSoundResourceRet, QStringList& vsTagsRet)
{

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();

  QTimer timer;
  timer.setSingleShot(false);
  timer.setInterval(1000);
  QEventLoop loop;
  QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);

  QMetaObject::Connection showRetValLoop =
      connect(pSignalEmitter, &::CTextBoxSignalEmitter::getDialogReturnValue,
          this, [this, sId, &sStringRet, &iTimeRet, &bWaitRet, &sSoundResourceRet, &vsTagsRet]
          (QString sRequestIdRet, QString sString, qint64 iTime, bool bWait, QString sResource, QStringList vsTags)
          {
            if (sId == sRequestIdRet)
            {
              sStringRet = sString;
              sStringRet.detach(); // fixes some crashes with QJSEngine
              iTimeRet = iTime;
              bWaitRet = bWait;
              sSoundResourceRet = sResource;
              sSoundResourceRet.detach(); // fixes some crashes with QJSEngine
              vsTagsRet = vsTags;
              vsTagsRet.detach(); // fixes some crashes with QJSEngine
              emit this->SignalQuitLoop();
            }
            // direct connection to fix cross thread issues with QString content being deleted
          }, Qt::DirectConnection);

  // connect lambdas in loop context, so events are processed, but capture timer,
  // to start / stop
  QMetaObject::Connection pauseLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::pauseExecution, &loop, [&timer]() {
            timer.stop();
          }, Qt::QueuedConnection);
  QMetaObject::Connection resumeLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::resumeExecution, &loop, [&timer]() {
            timer.start();
          }, Qt::QueuedConnection);

  bool bOk = true;
  QMetaObject::Connection timeoutLoop =
      connect(&timer, &QTimer::timeout, &loop, [this, &loop, &bOk]() {
        bOk = false;
        emit m_pSignalEmitter->showError("Timeout waiting for dialog request.", QtMsgType::QtWarningMsg);
        emit loop.exit();
      });

  QTimer::singleShot(0, this, [&pSignalEmitter, sId, sString, vsTags]() { emit pSignalEmitter->getDialog(sId, sString, vsTags); });

  timer.start();
  loop.exec();
  timer.stop();
  timer.disconnect();
  loop.disconnect();

  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(pauseLoop);
  disconnect(resumeLoop);
  disconnect(timeoutLoop);

  return bOk;
}

//----------------------------------------------------------------------------------------
//
class CCommandEosChoice : public CCommandEosChoiceBase
{
public:
  CCommandEosChoice(CEosScriptTextBox* pParent) :
    CCommandEosChoiceBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosChoice() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itOptions = GetValue<EArgumentType::eArray>(args, "options");
      if (HasValue(args, "options") && IsOk<EArgumentType::eArray>(itOptions))
      {
        QStringList vsOptions;
        std::map<qint32, qint32> vsOptionMapping;
        std::vector<QColor> vColors;
        std::vector<QColor> vColorsBg;
        std::vector<qint32> viNumChildren;
        qint32 iCounter = 0;

        const tInstructionArrayValue& arrOptions = std::get<tInstructionArrayValue>(itOptions);
        for (size_t i = 0; arrOptions.size() > i; ++i)
        {
          const auto& itOption = GetValue<EArgumentType::eMap>(arrOptions, i);
          if (IsOk<EArgumentType::eMap>(itOption))
          {
            const tInstructionMapValue& optionsArg = std::get<tInstructionMapValue>(itOption);

            const auto& itLabel = GetValue<EArgumentType::eString>(optionsArg, "label");
            const auto& itCommands = GetValue<EArgumentType::eArray>(optionsArg, "commands");
            const auto& itColor = GetValue<EArgumentType::eString>(optionsArg, "color");
            const auto& itVisible = GetValue<EArgumentType::eBool>(optionsArg, "visible");

            QString sLabel;
            if (HasValue(optionsArg, "label") && IsOk<EArgumentType::eString>(itLabel))
            {
              sLabel = std::get<QString>(itLabel);
              if (Qt::mightBeRichText(sLabel))
              {
                sLabel = "<html><body>" + sLabel + "</body></html>";
              }
            }
            if (HasValue(optionsArg, "commands") && IsOk<EArgumentType::eArray>(itCommands))
            {
              const tInstructionArrayValue& commands = std::get<tInstructionArrayValue>(itCommands);
              viNumChildren.push_back(static_cast<qint32>(commands.size()));
            }
            QColor col = Qt::black;
            if (HasValue(optionsArg, "color") && IsOk<EArgumentType::eString>(itColor))
            {
              col = std::get<QString>(itColor);
            }
            bool bVisible = true;
            if (HasValue(optionsArg, "visible") && IsOk<EArgumentType::eBool>(itVisible))
            {
              bVisible = std::get<bool>(itVisible);
            }

            if (bVisible)
            {
              // calculate foreground / text color
              double dLuminance = (0.299 * col.red() +
                                   0.587 * col.green() +
                                   0.114 * col.blue()) / 255;
              QColor foregroundColor = Qt::white;
              if (dLuminance > 0.5)
              {
                foregroundColor = Qt::black;
              }

              vsOptions.push_back(sLabel);
              vColors.push_back(foregroundColor);
              vColorsBg.push_back(col);
              vsOptionMapping.insert({iCounter++, static_cast<qint32>(i)});
            }
            else
            {
              vsOptionMapping.insert({static_cast<qint32>(1000000+i), static_cast<qint32>(i)});
            }
          }
        }

        m_pParent->setBackgroundColors(vColorsBg);
        m_pParent->setTextColors(vColors);
        qint32 iButton = m_pParent->showButtonPrompts(vsOptions);
        m_pParent->setBackgroundColors({QColor(Qt::black)});
        m_pParent->setTextColors({QColor(Qt::white)});

        qint32 iSiblingToCall = 0;
        qint32 iNumChildrenToCall = -1;
        qint32 iActualButton = vsOptionMapping[iButton];
        for (qint32 i = 0; i < iActualButton; ++i)
        {
          iSiblingToCall += viNumChildren[static_cast<size_t>(i)];
        }
        if (static_cast<qint32>(viNumChildren.size()) > iActualButton)
        {
          iNumChildrenToCall = viNumChildren[static_cast<size_t>(iActualButton)];
        }

        if (static_cast<qint32>(vsOptionMapping.size()) > 0 &&
            viNumChildren.size() > 0)
        {
          return SRunRetVal<ENextCommandToCall::eChild>(iSiblingToCall,
                                                        iSiblingToCall+iNumChildrenToCall);
        }
        else
        {
          return SRunRetVal<ENextCommandToCall::eSibling>();
        }
      }
    }
    return SJsonException{"internal Error.", "", eos::c_sCommandChoice, 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosPrompt : public CCommandEosPromptBase
{
public:
  CCommandEosPrompt(CEosScriptTextBox* pParent) :
    CCommandEosPromptBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosPrompt() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itVariable = GetValue<EArgumentType::eString>(args, "variable");
      if (HasValue(args, "variable") && IsOk<EArgumentType::eString>(itVariable))
      {
        const QString sVariable = std::get<QString>(itVariable);
        const QString sValue = m_pParent->showInput(sVariable);
        Q_UNUSED(sValue)

        return SRunRetVal<ENextCommandToCall::eSibling>();
      }
    }
    return SJsonException{"internal Error.", "", eos::c_sCommandPrompt, 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosSay : public CCommandEosSayBase
{
public:
  CCommandEosSay(CEosScriptTextBox* pParent) :
    CCommandEosSayBase(),
    m_pParent(pParent)
  {}
  ~CCommandEosSay() override {}

  IJsonInstructionBase::tRetVal Call(const tInstructionMapValue& args) override
  {
    if (nullptr != m_pParent)
    {
      const auto& itLabel = GetValue<EArgumentType::eString>(args, "label");
      const auto& itAlign = GetValue<EArgumentType::eString>(args, "align");
      const auto& itMode = GetValue<EArgumentType::eString>(args, "mode");
      const auto& itAllowSkip = GetValue<EArgumentType::eBool>(args, "allowSkip");
      const auto& itDuration = GetValue<EArgumentType::eString>(args, "duration");

      QString sLabel;
      if (HasValue(args, "label") && IsOk<EArgumentType::eString>(itLabel))
      {
        sLabel = "<html><body>" + std::get<QString>(itLabel) + "</body></html>";
      }

      if (HasValue(args, "align") && IsOk<EArgumentType::eString>(itAlign))
      {
        QString sAlign = std::get<QString>(itAlign);
        if (eos::c_vsAlignStrings[eos::eCenter] == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignCenter);
        }
        else if (eos::c_vsAlignStrings[eos::eLeft] == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignLeft);
        }
        else if (eos::c_vsAlignStrings[eos::eRight] == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignRight);
        }
      }

      CEosScriptTextBox::ESayMode mode = CEosScriptTextBox::eAutoplay;
      if (HasValue(args, "mode") && IsOk<EArgumentType::eString>(itMode))
      {
        QString sMode = std::get<QString>(itMode);
        if (eos::c_vsPlayModeStrings[eos::eAutoplay] == sMode)
        {
          mode = CEosScriptTextBox::eAutoplay;
        }
        else if (eos::c_vsPlayModeStrings[eos::eInstant] == sMode)
        {
          mode = CEosScriptTextBox::eInstant;
        }
        else if (eos::c_vsPlayModeStrings[eos::ePause] == sMode)
        {
          mode = CEosScriptTextBox::ePause;
        }
        else if (eos::c_vsPlayModeStrings[eos::eCustom] == sMode)
        {
          mode = CEosScriptTextBox::eCustom;
        }
      }

      bool bAllowSkip = false;
      if (HasValue(args, "allowSkip") && IsOk<EArgumentType::eBool>(itAllowSkip))
      {
        bAllowSkip = std::get<bool>(itAllowSkip);
      }

      qint64 iDurationMs = -1;
      if (HasValue(args, "itDuration") && IsOk<EArgumentType::eString>(itDuration))
      {
        iDurationMs = eos::ParseEosDuration(std::get<QString>(itDuration));
      }

      // based on mode we call things now
      if (CEosScriptTextBox::eAutoplay == mode)
      {
        iDurationMs = EstimateDurationBasedOnText(sLabel);
        if (0 == iDurationMs)
        {
          m_pParent->showText(sLabel);
        }
        else
        {
          m_pParent->showText(sLabel, static_cast<double>(iDurationMs) / 1000, bAllowSkip);
        }
      }
      else if (CEosScriptTextBox::eInstant == mode)
      {
        iDurationMs = 0;
        m_pParent->showText(sLabel);
      }
      else if (CEosScriptTextBox::ePause == mode)
      {
        m_pParent->showText(sLabel, std::numeric_limits<double>::max(), true);
      }
      else if (CEosScriptTextBox::eCustom == mode)
      {
        iDurationMs = EstimateDurationBasedOnText(sLabel);
        if (0 == iDurationMs)
        {
          m_pParent->showText(sLabel);
        }
        else
        {
          m_pParent->showText(sLabel, static_cast<double>(iDurationMs) / 1000, false || bAllowSkip);
        }
      }

      return SRunRetVal<ENextCommandToCall::eSibling>();
    }
    return SJsonException{"internal Error.", "", eos::c_sCommandSay, 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
};

//----------------------------------------------------------------------------------------
//
CEosScriptTextBox::CEosScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                                     QPointer<CJsonInstructionSetParser> pParser) :
  CEosScriptObjectBase(pEmitter, pParser),
  m_spCommandChoice(std::make_shared<CCommandEosChoice>(this)),
  m_spCommandPrompt(std::make_shared<CCommandEosPrompt>(this)),
  m_spCommandSay(std::make_shared<CCommandEosSay>(this))
{
  pParser->RegisterInstruction(eos::c_sCommandChoice, m_spCommandChoice);
  pParser->RegisterInstruction(eos::c_sCommandPrompt, m_spCommandPrompt);
  pParser->RegisterInstruction(eos::c_sCommandSay, m_spCommandSay);
}
CEosScriptTextBox::~CEosScriptTextBox()
{
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTextBox::setTextAlignment(qint32 alignment)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTextBoxSignalEmitter>()->textAlignmentChanged(alignment);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTextBox::setTextColors(const std::vector<QColor>& vColors)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTextBoxSignalEmitter>()->textColorsChanged(vColors);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTextBox::setBackgroundColors(const std::vector<QColor>& vColors)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTextBoxSignalEmitter>()->textBackgroundColorsChanged(vColors);
}

//----------------------------------------------------------------------------------------
//
qint32 CEosScriptTextBox::showButtonPrompts(const QStringList& vsLabels)
{
  if (!CheckIfScriptCanRun()) { return -1; }

  QString sRequestId = QUuid::createUuid().toString();

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  if (vsLabels.size() > 0)
  {
    emit pSignalEmitter->showButtonPrompts(vsLabels, QString(), sRequestId, false);

    // local loop to wait for answer
    qint32 iReturnValue = -1;
    QEventLoop loop;
    QMetaObject::Connection quitLoop =
      connect(this, &CEosScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
    QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection showRetValLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::showButtonReturnValue,
              this, [this, &iReturnValue, sRequestId](qint32 iIndexSelected, QString sRequestIdRet)
    {
      if (sRequestId == sRequestIdRet)
      {
        iReturnValue = iIndexSelected;
        emit this->SignalQuitLoop();
      }
    }, Qt::QueuedConnection);
    loop.exec();
    loop.disconnect();

    disconnect(quitLoop);
    disconnect(interruptLoop);
    disconnect(interruptThisLoop);
    disconnect(showRetValLoop);

    return iReturnValue;
  }

  return -1;
}

//----------------------------------------------------------------------------------------
//
QString CEosScriptTextBox::showInput(const QString& sStoreIntoVar)
{
  if (!CheckIfScriptCanRun()) { return QString(); }

  QString sRequestId = QUuid::createUuid().toString();

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter,sStoreIntoVar,sRequestId]() {
    emit pSignalEmitter->showInput(sStoreIntoVar, sRequestId, false);
  });

  // local loop to wait for answer
  QString sReturnValue = QString();
  QEventLoop loop;
  QMetaObject::Connection quitLoop =
    connect(this, &CEosScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
  QMetaObject::Connection interruptLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection interruptThisLoop =
    connect(this, &CScriptObjectBase::SignalInterruptExecution,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QMetaObject::Connection showRetValLoop =
    connect(pSignalEmitter, &CTextBoxSignalEmitter::showInputReturnValue,
            this, [this, &sReturnValue,sRequestId](QString sInput, QString sRequestIdRet)
  {
    if (sRequestId == sRequestIdRet)
    {
      sReturnValue = sInput;
      sReturnValue.detach(); // fixes some crashes with QJSEngine
      emit this->SignalQuitLoop();
    }
    // direct connection to fix cross thread issues with QString content being deleted
  }, Qt::DirectConnection);
  loop.exec();
  loop.disconnect();

  disconnect(quitLoop);
  disconnect(interruptLoop);
  disconnect(interruptThisLoop);
  disconnect(showRetValLoop);

  return sReturnValue;
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTextBox::showText(QString sText)
{
  if (!CheckIfScriptCanRun()) { return; }
  emit SignalEmitter<CTextBoxSignalEmitter>()->showText(sText, -1);
}

//----------------------------------------------------------------------------------------
//
void CEosScriptTextBox::showText(QString sText, double dWaitTime, bool bSkipable)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();

  if (0 < dWaitTime)
  {
    QDateTime lastTime = QDateTime::currentDateTime();
    qint32 iTimeLeft = dWaitTime * 1000;

    QTimer timer;
    timer.setSingleShot(false);
    timer.setInterval(20);
    QEventLoop loop;
    QMetaObject::Connection interruptLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);
    QMetaObject::Connection interruptThisLoop =
      connect(this, &CScriptObjectBase::SignalInterruptExecution,
              &loop, &QEventLoop::quit, Qt::QueuedConnection);

    // connect lambdas in loop context, so events are processed, but capture timer,
    // to start / stop
    QMetaObject::Connection pauseLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::pauseExecution, &loop, [&timer]() {
        timer.stop();
      }, Qt::QueuedConnection);
    QMetaObject::Connection resumeLoop =
      connect(pSignalEmitter, &CTextBoxSignalEmitter::resumeExecution, &loop, [&timer]() {
        timer.start();
      }, Qt::QueuedConnection);

    QMetaObject::Connection timeoutLoop =
      connect(&timer, &QTimer::timeout, &loop, [&loop, &iTimeLeft, &lastTime]() {
        QDateTime newTime = QDateTime::currentDateTime();
        iTimeLeft -= newTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch();
        lastTime = newTime;
        if (0 >= iTimeLeft)
        {
          emit loop.exit();
        }
      });

    QMetaObject::Connection skipLoop;
    if (bSkipable)
    {
      skipLoop =
        connect(pSignalEmitter, &CTextBoxSignalEmitter::waitSkipped,
                &loop, &QEventLoop::quit, Qt::QueuedConnection);
    }

    emit pSignalEmitter->showText(sText, bSkipable ? dWaitTime : 0);

    timer.start();
    loop.exec();
    timer.stop();
    timer.disconnect();
    loop.disconnect();

    disconnect(interruptLoop);
    disconnect(interruptThisLoop);
    disconnect(pauseLoop);
    disconnect(resumeLoop);
    disconnect(timeoutLoop);
    if (bSkipable)
    {
      disconnect(skipLoop);
    }
  }
}
