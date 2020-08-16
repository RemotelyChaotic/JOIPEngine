#include "ScriptTextBox.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include <QEventLoop>
#include <QDebug>

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

//----------------------------------------------------------------------------------------
//
CScriptTextBox::CScriptTextBox(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                               QPointer<QJSEngine> pEngine) :
  CScriptObjectBase(pEmitter, pEngine),
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
      CResource* pResource = dynamic_cast<CResource*>(resource.toQObject());
      if (nullptr != pResource)
      {
        tspResource spResource = pResource->Data();
        if (nullptr != spResource)
        {
          emit spSignalEmitter->textPortraitChanged(pResource->getName());
        }
        else
        {
          QString sError = tr("Resource in show() holds no data.");
          emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
        }
      }
      else
      {
        QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
        emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
      }
    }
    else if (resource.isNull() || resource.isUndefined())
    {
      emit spSignalEmitter->textPortraitChanged(QString());
    }
    else
    {
      QString sError = tr("Wrong argument-type to show(). String or resource was expected.");
      emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
    }
  }
}

//----------------------------------------------------------------------------------------
//
qint32 CScriptTextBox::showButtonPrompts(QJSValue vsLabels)
{
  if (!CheckIfScriptCanRun()) { return -1; }

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  if (vsLabels.isArray())
  {
    QStringList vsStringLabels;
    const qint32 iLength = vsLabels.property("length").toInt();
    for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
    {
      vsStringLabels << vsLabels.property(static_cast<quint32>(iIndex)).toString();
    }
    emit pSignalEmitter->showButtonPrompts(vsStringLabels);

    // local loop to wait for answer
    qint32 iReturnValue = -1;
    QEventLoop loop;
    connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
    connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
            &loop, &QEventLoop::quit, Qt::QueuedConnection);
    connect(pSignalEmitter, &CTextBoxSignalEmitter::showButtonReturnValue,
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
    emit m_pSignalEmitter->showError(sError, QtMsgType::QtWarningMsg);
  }

  return -1;
}

//----------------------------------------------------------------------------------------
//
void CScriptTextBox::showText(QString sText)
{
  if (!CheckIfScriptCanRun()) { return; }

  emit SignalEmitter<CTextBoxSignalEmitter>()->showText(sText);
}

//----------------------------------------------------------------------------------------
//
QString CScriptTextBox::showInput()
{
  if (!CheckIfScriptCanRun()) { return QString(); }

  auto pSignalEmitter = SignalEmitter<CTextBoxSignalEmitter>();
  emit pSignalEmitter->showInput();

  // local loop to wait for answer
  QString sReturnValue = QString();
  QEventLoop loop;
  connect(this, &CScriptTextBox::SignalQuitLoop, &loop, &QEventLoop::quit);
  connect(pSignalEmitter, &CTextBoxSignalEmitter::interrupt,
          &loop, &QEventLoop::quit, Qt::QueuedConnection);
  connect(pSignalEmitter, &CTextBoxSignalEmitter::showInputReturnValue,
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
