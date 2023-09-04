#include "CommonCodeGenerator.h"
#include "Application.h"
#include "Systems/DatabaseManager.h"
#include "Systems/DatabaseInterface/ResourceData.h"

CCommonCodeGenerator::CCommonCodeGenerator(const SCommonCodeConfiguration& codeConfig) :
  ICodeGenerator(),
  m_codeConfig(codeConfig)
{
}
CCommonCodeGenerator::~CCommonCodeGenerator()
{
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SBackgroundSnippetData& data,
                                       tspProject spCurrentProject) const
{
  Q_UNUSED(spCurrentProject)

  QString sCode;
  if (data.m_bUseColor)
  {
    QString sColor =
        Statement(Invoke("background", "setBackgroundColor") +
                  Call(Array("%1,%2,%3,%4")));
    sCode += sColor.arg(data.m_color.red()).arg(data.m_color.green())
        .arg(data.m_color.blue()).arg(data.m_color.alpha());
  }
  if (data.m_bUseResource)
  {
    QString sTexture =
        Statement(Invoke("background", "setBackgroundTexture") +
                  Call(String("%1")));
    sCode += sTexture.arg(data.m_sCurrentResource);
  }
  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SIconSnippetData& data,
                                       tspProject spCurrentProject) const
{
  Q_UNUSED(spCurrentProject)

  QString sCode;
  if (data.m_bShow)
  {
    QString sResource = Statement(Invoke("icon", "show") + Call(String("%1")));
    sCode += sResource.arg(data.m_sCurrentResource);
  }
  else
  {
    if (!data.m_sCurrentResource.isNull() && !data.m_sCurrentResource.isEmpty())
    {
      QString sResource = Statement(Invoke("icon", "hide") + Call(String("%1")));
      sCode += sResource.arg(data.m_sCurrentResource);
    }
    else
    {
      QString sResource = Statement(Invoke("icon", "hide") + Call(String("~all")));
      sCode += sResource;
    }
  }
  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SMetronomeSnippetCode& data,
                                       tspProject spCurrentProject) const
{
  QString sCode;
  if (data.m_bSetBpm)
  {
    QString sText = Statement(Invoke("metronome", "setBpm") + Call("%1"));
    sCode += sText.arg(data.m_iBpm);
  }
  if (data.m_bSetPattern)
  {
    QString sText = Statement(Invoke("metronome", "setPattern") + Call(Array("%1")));
    QStringList vsPatternElems;
    for (auto it = data.m_vdPatternElems.begin(); data.m_vdPatternElems.end() != it; ++it)
    {
      vsPatternElems << QString::number(it->second);
    }
    sCode += sText.arg(vsPatternElems.join(","));
  }

  sCode += Statement(Invoke("metronome", "setMuted") + Call("%1"))
      .arg(data.m_bSetMute ? m_codeConfig.sTrue : m_codeConfig.sFalse);

  if (data.m_bSetBeatSound)
  {
    EResourceType type = EResourceType::eSound;
    if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
    {
      if (nullptr != spCurrentProject)
      {
        tspResource spResource =
            spDbManager->FindResourceInProject(spCurrentProject, data.m_sBeatSound);
        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          type = spResource->m_type;
        }
      }
    }
    sCode +=
        Statement(Invoke("metronome", "setBeatResource") + Call("%1"))
        .arg((data.m_sBeatSound.isNull() || type._to_integral() != EResourceType::eSound) ?
             m_codeConfig.sNull :
             String(data.m_sBeatSound));
  }
  if (data.m_bSetVolume)
  {
    sCode +=
        Statement(Invoke("metronome", "setVolume") + Call("%1"))
        .arg(data.m_dVolume);
  }
  if (data.m_bStart)
  {
    sCode += Statement(Invoke("metronome", "start") + Call(""));
  }
  if (data.m_bStop)
  {
    sCode += Statement(Invoke("metronome", "stop") + Call(""));
  }
  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SNotificationSnippetCode& data,
                                       tspProject spCurrentProject) const
{
  QString sCode;
  if (data.m_bSetTextColor)
  {
    QString sText = Statement(Invoke("notification", "setTextColor") + Call("%1"));
    QString sColor = Array(
        QString::number(data.m_textColor.red()) + "," +
        QString::number(data.m_textColor.green()) + "," +
        QString::number(data.m_textColor.blue()) + "," +
        QString::number(data.m_textColor.alpha()));
    sCode += sText.arg(sColor);
  }
  if (data.m_bSetTextBackgroundColor)
  {
    QString sText = Statement(Invoke("notification", "setTextBackgroundColor") + Call("%1"));
    QString sColor = Array(
        QString::number(data.m_textBackgroundColor.red()) + "," +
        QString::number(data.m_textBackgroundColor.green()) + "," +
        QString::number(data.m_textBackgroundColor.blue()) + "," +
        QString::number(data.m_textBackgroundColor.alpha()));
    sCode += sText.arg(sColor);
  }
  if (data.m_bSetWidgetTextColors)
  {
    QString sText = Statement(Invoke("notification", "setWidgetColor") + Call("%1"));
    QString sColor = Array(
        QString::number(data.m_widgetTextColor.red()) + "," +
        QString::number(data.m_widgetTextColor.green()) + "," +
        QString::number(data.m_widgetTextColor.blue()) + "," +
        QString::number(data.m_widgetTextColor.alpha()));
    sCode += sText.arg(sColor);
  }
  if (data.m_bSetWidgetTextBackgroundColor)
  {
    QString sText = Statement(Invoke("notification", "setWidgetBackgroundColor") + Call("%1"));
    QString sColor = Array(
        QString::number(data.m_widgettextBackgroundColor.red()) + "," +
        QString::number(data.m_widgettextBackgroundColor.green()) + "," +
        QString::number(data.m_widgettextBackgroundColor.blue()) + "," +
        QString::number(data.m_widgettextBackgroundColor.alpha()));
    sCode += sText.arg(sColor);
  }
  if (data.m_bSetAlignment)
  {
    QString sText = Statement(Invoke("notification", "setIconAlignment") +
                              Call(Member("IconAlignment", "%1")));
    switch (data.m_textAlignment)
    {
    case Qt::AlignLeft: sText = sText.arg("AlignLeft"); break;
    case Qt::AlignRight: sText = sText.arg("AlignRight"); break;
    case Qt::AlignHCenter: sText = sText.arg("AlignCenter"); break;
    default: break;
    }
    sCode += sText;
  }
  if (data.m_bShowIcon)
  {
    EResourceType type = EResourceType::eImage;
    if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
    {
      if (nullptr != spCurrentProject)
      {
        tspResource spResource =
            spDbManager->FindResourceInProject(spCurrentProject, data.m_sIcon);
        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          type = spResource->m_type;
        }
      }
    }

    QString sText = Statement(Invoke("notification", "setPortrait") + Call("%2"));
    switch (type)
    {
      case EResourceType::eMovie: // fallthrough
      case EResourceType::eImage:
        sText = sText.arg(data.m_sIcon.isEmpty() ? m_codeConfig.sNull :
                                                   String(data.m_sIcon));
        break;
      case EResourceType::eSound: // fallthrough
    default: sText = ""; break;
    }

    sCode += sText;
  }
  QString sText;
  switch(data.m_displayStatus)
  {
    case EDisplayStatus::eShow:
    {
      QString sOptional;
      if (data.m_bSetTimeoutTime)
      {
        sOptional += QString(",%1").arg(data.m_dTimeoutTimeS);
      }
      if (data.m_bOnButton)
      {
        sOptional += QString(QString(",") + String("%1")).arg(data.m_sOnButton);
      }
      if (data.m_bOnTimeout)
      {
        if (!data.m_bSetTimeoutTime) { sOptional += ",-1"; }
        if (!data.m_bOnButton) { sOptional += "," + String(""); }
        sOptional += QString(","+String("%1")).arg(data.m_sOnTimeout);
      }
      sText = Statement(Invoke("notification", "show") + Call(String("%1") + "," +
                                                              String("%2") + "," +
                                                              String("%3") + "%4"))
          .arg(data.m_sId).arg(data.m_sText).arg(data.m_sWidgetText).arg(sOptional);
    } break;
    case EDisplayStatus::eHide:
      sText =
          Statement(Invoke("notification", "hide") + Call(String("%1"))).arg(data.m_sId);
      break;
    case EDisplayStatus::eClear:
      sText = Statement(Invoke("notification", "clear") + Call(""));
      break;
    default: break;
  }
  sCode += sText;

  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SResourceSnippetData& data,
                                       tspProject spCurrentProject) const
{
  EResourceType type = EResourceType::eImage;
  if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
  {
    if (nullptr != spCurrentProject)
    {
      tspResource spResource =
          spDbManager->FindResourceInProject(spCurrentProject, data.m_sResource);
      if (nullptr != spResource)
      {
        QReadLocker locker(&spResource->m_rwLock);
        type = spResource->m_type;
      }
    }
  }

  QString sAlias = "";
  if (EResourceType::eSound == type._to_integral())
  {
    sAlias = String("") + ",";
  }
  QString sLoopsAndStart;
  if (data.m_bLoops)
  {
    sLoopsAndStart = (data.m_bStartAt || data.m_iEndAt) ? ",%3 %1, %2" : ",%2 %1";
    if (data.m_bStartAt)
    {
      sLoopsAndStart = sLoopsAndStart.arg(data.m_iLoops)
          .arg(!data.m_bEndAt ? QString::number(data.m_iStartAt) :
                                QString("%1, %2").arg(data.m_bStartAt ? data.m_iStartAt : 0).arg(data.m_iEndAt))
          .arg(sAlias);
    }
    else
    {
      sLoopsAndStart = sLoopsAndStart
          .arg(data.m_iLoops)
          .arg(sAlias);
    }
  }
  else if (data.m_bStartAt || data.m_bEndAt)
  {
    sLoopsAndStart = ",%2 1, %1";
    sLoopsAndStart = sLoopsAndStart
        .arg(!data.m_bEndAt ? QString::number(data.m_iStartAt) :
                              QString("%1, %2").arg(data.m_bStartAt ? data.m_iStartAt : 0).arg(data.m_iEndAt))
        .arg(sAlias);
  }


  QString sCode;
  if (!data.m_sResource.isEmpty())
  {
    if (EDisplayMode::ePlayShow == data.m_displayMode)
    {
      QString sMainCommand =
          Statement(Invoke("mediaPlayer", "%1") + Call(String("%2") + "%3"));
      switch (type)
      {
        case EResourceType::eImage:
          sMainCommand = sMainCommand.arg("show");
          break;
        case EResourceType::eMovie:
          sMainCommand = sMainCommand.arg("play");
          break;
        case EResourceType::eSound:
          sMainCommand = sMainCommand.arg("playSound");
          break;
        default: break;
      }
      sMainCommand = sMainCommand.arg(data.m_sResource);
      if (EResourceType::eMovie == type._to_integral() || EResourceType::eSound == type._to_integral())
      {
        sMainCommand = sMainCommand.arg(sLoopsAndStart);
      }
      else if (EResourceType::eImage == type._to_integral())
      {
        sMainCommand = sMainCommand.arg("");
      }
      sCode += sMainCommand;
    }
    else if (EDisplayMode::ePause == data.m_displayMode)
    {
      switch (type)
      {
        case EResourceType::eImage: break;
        case EResourceType::eMovie:
          sCode += Statement(Invoke("mediaPlayer", "pauseVideo") + Call("")); break;
        case EResourceType::eSound:
          sCode += Statement(Invoke("mediaPlayer", "pauseSound") + Call(String("%1")))
              .arg(data.m_sResource);
          break;
        default: break;
      }
    }
    else if (EDisplayMode::eStop == data.m_displayMode)
    {
      switch (type)
      {
        case EResourceType::eImage: break;
        case EResourceType::eMovie:
          sCode += Statement(Invoke("mediaPlayer", "stopVideo") + Call("")); break;
        case EResourceType::eSound:
          sCode += Statement(Invoke("mediaPlayer", "stopSound") + Call(String("%1")))
              .arg(data.m_sResource);
          break;
        default: break;
      }
    }
    else if (EDisplayMode::eSeek == data.m_displayMode)
    {
      switch (type)
      {
        case EResourceType::eImage: break;
        case EResourceType::eMovie:
          sCode += Statement(Invoke("mediaPlayer", "seekVideo") + Call("%1"))
              .arg(data.m_iSeekTime);
          break;
        case EResourceType::eSound:
          sCode += Statement(Invoke("mediaPlayer", "seekSound") + Call(String("%1") + ",%2"))
              .arg(data.m_sResource).arg(data.m_iSeekTime);
          break;
        default: break;
      }
    }
  }
  else
  {
    if (EDisplayMode::ePlayShow == data.m_displayMode)
    {
      sCode += Statement(Invoke("mediaPlayer", "play") + Call(""));
    }
    else if (EDisplayMode::ePause == data.m_displayMode)
    {
      sCode += Statement(Invoke("mediaPlayer", "pauseVideo") + Call(""));
      sCode += Statement(Invoke("mediaPlayer", "pauseSound") + Call(""));
    }
    else if (EDisplayMode::eStop == data.m_displayMode)
    {
      sCode += Statement(Invoke("mediaPlayer", "stopVideo") + Call(""));
      sCode += Statement(Invoke("mediaPlayer", "stopSound") + Call(""));
    }
    else if (EDisplayMode::eSeek == data.m_displayMode)
    {
      sCode += Statement(Invoke("mediaPlayer", "seekVideo") + Call("%1"))
          .arg(data.m_iSeekTime);
    }
  }

  if ((type._to_integral() == EResourceType::eMovie || type._to_integral() == EResourceType::eSound)
      && data.m_bWaitForFinished)
  {
    if (!data.m_sResource.isEmpty())
    {
      switch (type)
      {
        case EResourceType::eImage:
          sCode += Statement(Invoke("mediaPlayer", "waitForPlayback") + Call(String("%1")))
              .arg(data.m_sResource);
          break;
        case EResourceType::eMovie:
          sCode += Statement(Invoke("mediaPlayer", "waitForVideo") + Call(""));
          break;
        case EResourceType::eSound:
          sCode += Statement(Invoke("mediaPlayer", "waitForSound") + Call(String("%1")))
              .arg(data.m_sResource);
          break;
        default: break;
      }
    }
    else
    {
      sCode += Statement(Invoke("mediaPlayer", "waitForPlayback") + Call(""));
    }
  }

  if (data.m_bSetVolume &&
      (type._to_integral() == EResourceType::eMovie || type._to_integral() == EResourceType::eSound))
  {
    sCode +=
        Statement(Invoke("mediaPlayer", "setVolume") + Call(String("%1") + ", %2"))
        .arg(data.m_sResource).arg(data.m_dVolume);
  }

  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const STextSnippetCode& data,
                                       tspProject spCurrentProject) const
{
  QString sCode;
  if (data.m_bSetTextColors)
  {
    QString sText = Statement(Invoke("textBox", "setTextColors") + Call(Array("%1")));
    QStringList vsColors;
    for (auto it : data.m_vTextColors)
    {
      QString sColor = Array(QString::number(it.second.red()) + "," +
          QString::number(it.second.green()) + "," +
          QString::number(it.second.blue()));
      vsColors << sColor;
    }
    sCode += sText.arg(vsColors.join(","));
  }
  if (data.m_bSetBGColors)
  {
    QString sText = Statement(Invoke("textBox", "setBackgroundColors") + Call(Array("%1")));
    QStringList vsColors;
    for (auto it : data.m_vBGColors)
    {
      QString sColor = Array(QString::number(it.second.red()) + "," +
          QString::number(it.second.green()) + "," +
          QString::number(it.second.blue()));
      vsColors << sColor;
    }
    sCode += sText.arg(vsColors.join(","));
  }
  if (data.m_bSetAlignment)
  {
    QString sText = Statement(Invoke("textBox", "setTextAlignment") +
                              Call(Member("TextAlignment", "%1")));
    switch (data.m_textAlignment)
    {
    case Qt::AlignLeft: sText = sText.arg("AlignLeft"); break;
    case Qt::AlignRight: sText = sText.arg("AlignRight"); break;
    case Qt::AlignHCenter: sText = sText.arg("AlignCenter"); break;
    default: break;
    }
    sCode += sText;
  }
  if (data.m_bShowIcon)
  {
    EResourceType type = EResourceType::eImage;
    if (auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock())
    {
      if (nullptr != spCurrentProject)
      {
        tspResource spResource =
            spDbManager->FindResourceInProject(spCurrentProject, data.m_sTextIcon);
        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          type = spResource->m_type;
        }
      }
    }

    QString sText = Statement(Invoke("textBox", "setTextPortrait") + Call("%2"));
    switch (type)
    {
      case EResourceType::eMovie: // fallthrough
      case EResourceType::eImage:
        sText = sText.arg(data.m_sTextIcon.isEmpty() ? m_codeConfig.sNull :
                                                       String(data.m_sTextIcon));
        break;
      case EResourceType::eSound: // fallthrough
    default: sText = ""; break;
    }

    sCode += sText;
  }
  if (data.m_bShowText)
  {
    QString sOptionalArgs;
    if (data.m_bSetSleepTime)
    {
      double dSleepTimeS = data.m_dSleepTimeS;
      if (data.m_bAutoTime) { dSleepTimeS = -1; }
      sOptionalArgs = QString(", %1, %2")
          .arg(dSleepTimeS).arg(data.m_bSkippable ? m_codeConfig.sTrue : m_codeConfig.sFalse);
    }

    QString sText = Statement(Invoke("textBox", "showText") + Call(String("%1") + "%2"));
    sCode += sText.arg(data.m_sText).arg(sOptionalArgs);
  }
  if (data.m_bShowUserInput)
  {
    QString sText = Statement(Assignment(Local("sInput"),
                                         Invoke("textBox", "showInput") + Call("")),
                              Comment("TODO: change variable name"));
    sCode += sText;
  }
  if (data.m_bShowButtons)
  {
    QString sText = Statement(Assignment(Local("iSelection"),
                                         Invoke("textBox", "showButtonPrompts") + Call(Array("%1"))),
                              Comment("TODO: change variable name"));
    QStringList vsPrompts;
    for (auto sButton : data.m_vsButtons)
    {
      vsPrompts << String(sButton);
    }
    sCode += sText.arg(vsPrompts.join(","));
  }

  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const SThreadSnippetOverlay& data,
                                       tspProject spCurrentProject) const
{
  Q_UNUSED(spCurrentProject)
  QString sCode = Statement(Invoke("thread", "sleep") + Call("%1,%2"))
      .arg(data.m_bSleepTimeS)
      .arg(data.m_bSkippable ? m_codeConfig.sTrue : m_codeConfig.sFalse);
  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Generate(const STimerSnippetData& data,
                                       tspProject spCurrentProject) const
{
  Q_UNUSED(spCurrentProject)
  QString sCode;

  if (data.m_bStop)
  {
    QString sTimer = Statement(Invoke("timer", "stop") + Call(""));
    sCode += sTimer;
  }
  if (data.m_bHide)
  {
    QString sTimer = Statement(Invoke("timer", "hide") + Call(""));
    sCode += sTimer;
  }

  QString sTimerVisible = Statement(Invoke("timer", "setTimeVisible") + Call("%1"));
  sCode += sTimerVisible.arg(data.m_bTimerVisible ? m_codeConfig.sTrue : m_codeConfig.sFalse);

  if (data.m_bSetTime)
  {
    QString sTimer = Statement(Invoke("timer", "setTime") + Call("%1"));
    sCode += sTimer.arg(data.m_iTimeS);
  }
  if (data.m_bShow)
  {
    QString sTimer = Statement(Invoke("timer", "show") + Call(""));
    sCode += sTimer;
  }
  if (data.m_bStart)
  {
    QString sTimer = Statement(Invoke("timer", "start") + Call(""));
    sCode += sTimer;
  }
  if (data.m_bWait)
  {
    QString sTimer = Statement(Invoke("timer", "waitForTimer") + Call(""));
    sCode += sTimer;
  }

  return sCode;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Array(const QString& sContent) const
{
  return QString(m_codeConfig.arrStart) + sContent + m_codeConfig.arrEnd;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Assignment(const QString& sLhs, const QString& sRhs) const
{
  return sLhs + " = " + sRhs;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Call(const QString& sContent) const
{
  return QString(m_codeConfig.callStart) + sContent + m_codeConfig.callEnd;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Comment(const QString& sContent) const
{
  return m_codeConfig.sComment + " " + sContent;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Invoke(const QString& sObj, const QString& sMember) const
{
  return sObj + m_codeConfig.invokationOp + sMember;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Member(const QString& sObj, const QString& sMember) const
{
  return sObj + m_codeConfig.memberOp + sMember;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Local(const QString& sVariable) const
{
  return m_codeConfig.sLocalKeyWord + " " + sVariable;
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::Statement(const QString& sContent, const QString& sBeforeNewLine) const
{
  return sContent + m_codeConfig.statementFinish + sBeforeNewLine + "\n";
}

//----------------------------------------------------------------------------------------
//
QString CCommonCodeGenerator::String(const QString& sContent) const
{
  return QString(m_codeConfig.stringChar) + sContent + m_codeConfig.stringChar;
}
