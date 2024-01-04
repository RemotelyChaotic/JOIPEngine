#ifndef ICODEGENERATOR_H
#define ICODEGENERATOR_H

#include "Systems/Project.h"
#include "Systems/DatabaseInterface/ResourceData.h"

#include <QString>
#include <QColor>
#include <map>
#include <memory>

struct SBackgroundSnippetData
{
  bool      m_bUseResource = false;
  QString   m_sCurrentResource = QString();
  bool      m_bUseColor = false;
  QColor    m_color = QColor();
};

struct SDeviceSnippetData
{
  bool   m_bVibrateCommand = false;
  double m_dVibrateSpeed = 0.0;
  bool   m_bLinearCommand = false;
  double m_dLinearDurationS = 0.0;
  double m_dLinearPosition = 0.0;
  bool   m_bRotateCommand = false;
  bool   m_bClockwiseRotate = true;
  double m_dRotateSpeed = 0.0;
  bool   m_bStopCommand = false;
};

struct SIconSnippetData
{
  bool      m_bShow = false;
  QString   m_sCurrentResource = QString();
};

struct SMetronomeSnippetCode
{
  bool m_bStart = true;
  bool m_bStop = false;
  bool m_bSetBpm = false;
  qint32 m_iBpm = 60;
  bool m_bSetPattern = false;
  std::map<qint32, double> m_vdPatternElems;
  bool m_bSetMute = false;
  bool m_bSetBeatSound = false;
  QString m_sBeatSound = QString();
  bool m_bSetVolume = false;
  double m_dVolume = 1.0;
};

enum EDisplayStatus
{
  eShow,
  eHide,
  eClear
};

struct SNotificationSnippetCode
{
  QString m_sId = QString();
  EDisplayStatus m_displayStatus = EDisplayStatus::eShow;
  QString m_sText = QString();
  QString m_sWidgetText = QString();
  bool m_bSetAlignment = false;
  Qt::AlignmentFlag m_textAlignment = Qt::AlignHCenter;
  bool m_bSetTimeoutTime = false;
  double m_dTimeoutTimeS = -1;
  bool m_bShowIcon = false;
  QString m_sIcon = QString();
  bool m_bOnButton = false;
  QString m_sOnButton = QString();
  bool m_bOnTimeout = false;
  QString m_sOnTimeout = QString();
  bool m_bSetTextColor = false;
  QColor m_textColor;
  bool m_bSetTextBackgroundColor = false;
  QColor m_textBackgroundColor;
  bool m_bSetWidgetTextColors = false;
  QColor m_widgetTextColor;
  bool m_bSetWidgetTextBackgroundColor = false;
  QColor m_widgettextBackgroundColor;
};

enum class EDisplayMode : qint32
{
  ePlayShow,
  ePause,
  eStop,
  eSeek
};

struct SResourceSnippetData
{
  QString m_sResource;
  EDisplayMode m_displayMode = EDisplayMode::ePlayShow;
  bool m_bWaitForFinished = false;
  qint32 m_iSeekTime = 0;
  bool m_bLoops = false;
  qint64 m_iLoops = 1;
  bool m_bStartAt = false;
  qint64 m_iStartAt = 0;
  bool m_bEndAt = false;
  qint64 m_iEndAt = -1;
  bool m_bSetVolume = false;
  double m_dVolume = 1.0;
};

struct STextSnippetCode
{
  bool m_bShowText = false;
  bool m_bShowUserInput = false;
  QString m_sText = QString();
  bool m_bSetAlignment = false;
  Qt::AlignmentFlag m_textAlignment = Qt::AlignHCenter;
  bool m_bSetSleepTime = false;
  bool m_bAutoTime = true;
  double m_dSleepTimeS = 0.0;
  bool m_bSkippable = false;
  bool m_bShowIcon = false;
  QString m_sTextIcon = QString();
  bool m_bShowButtons = false;
  std::vector<QString> m_vsButtons;
  bool m_bSetTextColors = false;
  std::map<qint32, QColor> m_vTextColors;
  bool m_bSetBGColors = false;
  std::map<qint32, QColor> m_vBGColors;
};

struct SThreadSnippetOverlay
{
  double m_bSleepTimeS;
  bool   m_bSkippable;
};

struct STimerSnippetData
{
  bool m_bSetTime = false;
  qint32 m_iTimeS = 0;
  bool m_bShow = false;
  bool m_bHide = false;
  bool m_bTimerVisible = false;
  bool m_bStart = false;
  bool m_bStop = false;
  bool m_bWait = false;
};

//----------------------------------------------------------------------------------------
//
class ICodeGenerator
{
public:
  ICodeGenerator() = default;
  virtual ~ICodeGenerator() = default;

  virtual QString Generate(const SBackgroundSnippetData& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SDeviceSnippetData& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SIconSnippetData& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SMetronomeSnippetCode& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SNotificationSnippetCode& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SResourceSnippetData& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const STextSnippetCode& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const SThreadSnippetOverlay& data, tspProject spCurrentProject) const = 0;
  virtual QString Generate(const STimerSnippetData& data, tspProject spCurrentProject) const = 0;
};

#endif // ICODEGENERATOR_H
