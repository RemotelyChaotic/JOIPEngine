#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include "Enums.h"
#include "ResourceData.h"
#include "SVersion.h"
#include <QString>
#include <map>
#include <memory>


enum EDownloadStateFlag : qint32
{
  eUnstarted        = 0x1,
  eDownloadRunning  = 0x2,
  eFinished         = 0x4
};

BETTER_ENUM(EDownLoadState, qint32,
            eUnstarted        = EDownloadStateFlag::eUnstarted,
            eDownloadRunning  = EDownloadStateFlag::eDownloadRunning,
            eFinished         = EDownloadStateFlag::eFinished)

Q_DECLARE_FLAGS(EDownLoadStateFlags, EDownloadStateFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(EDownLoadStateFlags)
Q_DECLARE_METATYPE(EDownLoadStateFlags)

struct SProjectData
{
  SProjectData() = default;
  SProjectData(const SProjectData& other) = default;

  qint32                    m_iId = -1;
  SVersion                  m_iVersion = SVersion(1, 0, 0);
  SVersion                  m_iTargetVersion = SVersion(VERSION_XYZ);
  QString                   m_sName;
  QString                   m_sFolderName;
  QString                   m_sProjectPath;
  QString                   m_sDescribtion;
  QString                   m_sTitleCard;
  QString                   m_sMap;
  QString                   m_sSceneModel;
  QString                   m_sPlayerLayout;
  ETutorialState            m_tutorialState = ETutorialState::eFinished;
  qint32                    m_iNumberOfSoundEmitters = 5;
  bool                      m_bUsesWeb = false;
  bool                      m_bNeedsCodecs = false;
  bool                      m_bBundled = false;
  bool                      m_bReadOnly = false;
  bool                      m_bLoaded = false;
  EDownLoadState            m_dlState = EDownLoadState::eFinished;
  QString                   m_sFont = "Segoe UI";
  QString                   m_sUserData;
};

#endif // PROJECTDATA_H
