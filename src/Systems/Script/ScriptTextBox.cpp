#include "ScriptTextBox.h"
#include "Application.h"
#include "ScriptRunnerSignalEmiter.h"
#include "Systems/DatabaseManager.h"
#include <QEventLoop>

CScriptTextBox::CScriptTextBox(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                               QJSEngine* pEngine) :
  QObject(),
  m_spSignalEmitter(spEmitter),
  m_wpDbManager(CApplication::Instance()->System<CDatabaseManager>()),
  m_pEngine(pEngine)
{
}

CScriptTextBox::~CScriptTextBox()
{
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setBackgroundColors(QJSValue colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setBackgroundColors()");
  emit m_spSignalEmitter->SignalTextBackgroundColorsChanged(colorsConverted);
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::setTextColors(QJSValue colors)
{
  if (!CheckIfScriptCanRun()) { return; }

  std::vector<QColor> colorsConverted = GetColors(colors, "setTextColors()");
  emit m_spSignalEmitter->SignalTextColorsChanged(colorsConverted);
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptTextBox::showButtonPrompts(QJSValue vsLabels)
{
  if (!CheckIfScriptCanRun()) { return -1; }

  if (vsLabels.isArray())
  {
    QStringList vsStringLabels;
    const qint32 iLength = vsLabels.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      vsStringLabels << vsLabels.property(static_cast<quint32>(iIndex)).toString();
    }
    emit m_spSignalEmitter->SignalShowButtonPrompts(vsStringLabels);

    // local loop to wait for answer
    qint32 iReturnValue = -1;
    QEventLoop loop;
    connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
    connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalShowButtonReturnValue,
            this, [this, &iReturnValue](qint32 iIndexSelected)
    {
      iReturnValue = iIndexSelected;
      emit this->SignalQuitLoop();
    }, Qt::QueuedConnection);
    loop.exec();
    loop.disconnect();

    return iReturnValue;
  }
  else
  {
    QString sError = tr("Wrong argument-type to showButtonPrompts(). String-array was expected.");
    emit m_spSignalEmitter->SignalShowError(sError, QtMsgType::QtWarningMsg);
  }

  return -1;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText)
{
  if (!CheckIfScriptCanRun()) { return; }

  emit m_spSignalEmitter->SignalShowText(sText);
}

//----------------------------------------------------------------------------------------
//
QString CScriptTextBox::showInput()
{
  if (!CheckIfScriptCanRun()) { return QString(); }

  emit m_spSignalEmitter->SignalShowInput();

  // local loop to wait for answer
  QString sReturnValue = QString();
  QEventLoop loop;
  connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalInterruptLoops,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(m_spSignalEmitter.get(), &CScriptRunnerSignalEmiter::SignalShowInputReturnValue,
          this, [this, &sReturnValue](QString sInput)
  {
    sReturnValue = sInput;
    emit this->SignalQuitLoop();
  }, Qt::QueuedConnection);
  loop.exec();
  loop.disconnect();

  return sReturnValue;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::clear()
{
  if (!CheckIfScriptCanRun()) { return; }

  emit m_spSignalEmitter->SignalClearText();
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

        if (viColorComponents.size() != 3)
        {
          QString sError = tr("Argument error in %1. Array of three numbers or string was expected.");
          emit m_spSignalEmitter->SignalShowError(sError.arg(sSource), QtMsgType::QtWarningMsg);
        }
        else
        {
          colorsRet.push_back(
                QColor(viColorComponents[0], viColorComponents[1], viColorComponents[2]));
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to %1. Array of three numbers or string was expected.");
        emit m_spSignalEmitter->SignalShowError(sError.arg(sSource), QtMsgType::QtWarningMsg);
      }
    }
  }
  else
  {
    QString sError = tr("Wrong argument-type to %1. Array of arrays of three numbers or array of strings was expected.");
    emit m_spSignalEmitter->SignalShowError(sError.arg(sSource), QtMsgType::QtWarningMsg);
  }

  return colorsRet;
}

//----------------------------------------------------------------------------------------
//
bool CScriptTextBox::CheckIfScriptCanRun()
{
  if (m_spSignalEmitter->ScriptExecutionStatus()._to_integral() == EScriptExecutionStatus::eStopped)
  {
    QJSValue val = m_pEngine->evaluate("f();"); //undefined function -> create error
    Q_UNUSED(val);
    return false;
  }
  else
  {
    return true;
  }
}
