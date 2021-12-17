#include "ScriptTextBox.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/EOS/EosHelpers.h"
#include "Systems/JSON/JsonInstructionBase.h"
#include "Systems/JSON/JsonInstructionSetParser.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"

#include <QDateTime>
#include <QDebug>
#include <QEventLoop>
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

//----------------------------------------------------------------------------------------
//
CScriptTextBox::CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<QJSEngine> pEngine) :
  CJsScriptObjectBase(pEmitter, pEngine),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>())
{
}

CScriptTextBox::~CScriptTextBox()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setBackgroundColors(QJSValue colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setBackgroundColors()");
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
void CScriptTextBox::setTextColors(QJSValue colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setTextColors()");
  emit SignalEmitter<CTextBoxSignalEmitter>()->textColorsChanged(colorsConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextPortrait(QJSValue resource)
{
  if (!CheckIfScriptCanRun()) { return; }

  auto spSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  auto spDbManager = m_wpDbManager.lock();
  if (nullptr != spDbManager)
  {
    if (resource.isString())
    {
      QString sResourceName = resource.toString();
      tspResource spResource = spDbManager->FindResourceInProject(m_spProject, sResourceName);
      if (nullptr != spResource)
      {
        emit spSignalEmitter->textPortraitChanged(sResourceName);
      }
      else
      {
        QString sError = tr("Resource %1 not found");
        emit m_pSignalEmitter->showError(sError.arg(resource.toString()),
                                                QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isQObject())
    {
      CResourceScriptWrapper* pResource = dynamic_cast<CResourceScriptWrapper*>(resource.toQObject());
      if (nullptr != pResource)
      {
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit spSignalEmitter->textPortraitChanged(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in setTextPortrait() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to setTextPortrait(). String or resource was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull() || resource.isUndefined())
    {
      emit spSignalEmitter->textPortraitChanged(QString());
    }
    else
    {
      QString sError = tr("Wrong argument-type to setTextPortrait(). String or resource was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptTextBox::showButtonPrompts(QJSValue vsLabels)
{
  if (!CheckIfScriptCanRun()) { return -1; }

  QString sRequestId = QUuid::createUuid().toString();

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  if (vsLabels.isArray())
  {
    QStringList vsStringLabels;
    const qint32 iLength = vsLabels.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      vsStringLabels << vsLabels.property(static_cast<quint32>(iIndex)).toString();
    }
    emit pSignalEmitter->showButtonPrompts(vsStringLabels, sRequestId);

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
  else
  {
    QString sError = tr("Wrong argument-type to showButtonPrompts(). String-array was expected.");
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
  }

  return -1;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText)
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CTextBoxSignalEmitter>()->showText(sText, -1);
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

    QTimer::singleShot(0, this, [&pSignalEmitter,sText,bSkipable,dWaitTime]() { emit pSignalEmitter->showText(sText, bSkipable ? dWaitTime : 0); });

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
  if (!CheckIfScriptCanRun()) { return QString(); }

  QString sRequestId = QUuid::createUuid().toString();
  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  QTimer::singleShot(0, this, [&pSignalEmitter, sRequestId]() {
    emit pSignalEmitter->showInput(QString(), sRequestId, true);
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
std::vector<QColor> CScriptTextBox::GetColors(const QJSValue& colors, const QString& sSource)
{
  std::vector<QColor> colorsRet;

  if (colors.isArray())
  {
    const qint32 iLength = colors.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      QJSValue color = colors.property(static_cast<quint32>(iIndex));

      if (color.isString())
      {
        colorsRet.push_back(QColor(color.toString()));
      }
      else if (color.isArray())
      {
        std::vector<qint32> viColorComponents;
        const qint32 iLength = color.property("length").toInt();
        for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
        {
          viColorComponents.push_back(color.property(static_cast<quint32>(iIndex)).toInt());
        }

        if (viColorComponents.size() != 4 && viColorComponents.size() != 3)
        {
          QString sError = tr("Argument error in %1. Array of three or four numbers or string was expected.");
          emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
        }
        else
        {
          if (viColorComponents.size() == 4)
          {
            colorsRet.push_back(
                  QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2], viColorComponents[3]));
          }
          else
          {
            colorsRet.push_back(
                  QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2]));
          }
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to %1. Array of three or four numbers or string was expected.");
        emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
      }
    }
  }
  else
  {
    QString sError = tr("Wrong argument-type to %1. Array of arrays of three or four numbers or array of strings was expected.");
    emit m_pSignalEmitter->showError(sError.arg(sSource), QtMsgType::QtWarningMsg);
  }

  return colorsRet;
}

//----------------------------------------------------------------------------------------
//
class CCommandEosChoice : public IJsonInstructionBase
{
public:
  CCommandEosChoice(CEosScriptTextBox* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"options", SInstructionArgumentType{EArgumentType::eArray,
               MakeArgArray(EArgumentType::eMap, tInstructionMapType{
                     {"label", SInstructionArgumentType{EArgumentType::eString}},
                     {"commands", SInstructionArgumentType{EArgumentType::eArray,
                            MakeArgArray(EArgumentType::eObject)}},
                     {"color", SInstructionArgumentType{EArgumentType::eString}},
                     {"visible", SInstructionArgumentType{EArgumentType::eBool}}
               })
      }},
    }) {}
  ~CCommandEosChoice() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
              sLabel = "<html><body>" + std::get<QString>(itLabel) + "</body></html>";
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
    return SJsonException{"internal Error.", "", "choice", 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosPrompt : public IJsonInstructionBase
{
public:
  CCommandEosPrompt(CEosScriptTextBox* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"variable", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosPrompt() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
    return SJsonException{"internal Error.", "", "prompt", 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
  tInstructionMapType    m_argTypes;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosSay : public IJsonInstructionBase
{
public:
  CCommandEosSay(CEosScriptTextBox* pParent) :
    m_pParent(pParent),
    m_argTypes({
      {"label", SInstructionArgumentType{EArgumentType::eString}},
      {"align", SInstructionArgumentType{EArgumentType::eString}},
      {"mode", SInstructionArgumentType{EArgumentType::eString}},
      {"allowSkip", SInstructionArgumentType{EArgumentType::eBool}},
      {"duration", SInstructionArgumentType{EArgumentType::eString}}
    }) {}
  ~CCommandEosSay() override {}

  tInstructionMapType& ArgList() override
  {
    return m_argTypes;
  }

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
        if ("center" == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignCenter);
        }
        else if ("left" == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignLeft);
        }
        else if ("right" == sAlign)
        {
          m_pParent->setTextAlignment(TextAlignment::AlignRight);
        }
      }

      CEosScriptTextBox::ESayMode mode = CEosScriptTextBox::eAutoplay;
      if (HasValue(args, "mode") && IsOk<EArgumentType::eString>(itMode))
      {
        QString sMode = std::get<QString>(itMode);
        if ("autoplay" == sMode)
        {
          mode = CEosScriptTextBox::eAutoplay;
        }
        else if ("instant" == sMode)
        {
          mode = CEosScriptTextBox::eInstant;
        }
        else if ("pause" == sMode)
        {
          mode = CEosScriptTextBox::ePause;
        }
        else if ("custom" == sMode)
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
    return SJsonException{"internal Error.", "", "say", 0, 0};
  }

private:
  CEosScriptTextBox*     m_pParent;
  tInstructionMapType    m_argTypes;
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
  pParser->RegisterInstruction("choice", m_spCommandChoice);
  pParser->RegisterInstruction("prompt", m_spCommandPrompt);
  pParser->RegisterInstruction("say", m_spCommandSay);
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
    emit pSignalEmitter->showButtonPrompts(vsLabels, sRequestId);

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
