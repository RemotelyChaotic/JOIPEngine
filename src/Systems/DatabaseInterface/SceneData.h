#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <enum.h>
#include <QString>
#include <set>

BETTER_ENUM(ESceneTransitionType, qint32,
            eRandom = 0,
            eSelection = 1)

BETTER_ENUM(ESceneMode, qint32,
            eLinear = 0,
            eEventDriven = 1)

typedef std::set<QString>        tvsResourceRefs;

struct SSceneData
{
  SSceneData() = default;
  SSceneData(const SSceneData& other) = default;

  qint32                    m_iId = -1;
  QString                   m_sName;
  QString                   m_sScript;
  QString                   m_sSceneLayout;
  bool                      m_bCanStartHere = false;
  ESceneMode                m_sceneMode = ESceneMode::eLinear;
  QString                   m_sTitleCard;

  tvsResourceRefs           m_vsResourceRefs; // unused
};

#endif // SCENEDATA_H
