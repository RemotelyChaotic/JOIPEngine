#ifndef SCENEDATA_H
#define SCENEDATA_H

#include <enum.h>
#include <QString>

BETTER_ENUM(ESceneTransitionType, qint32,
            eRandom = 0,
            eSelection = 1);

struct SSceneData
{
  SSceneData() = default;
  SSceneData(const SSceneData& other) = default;

  qint32                    m_iId;
  QString                   m_sName;
  QString                   m_sScript;
  QString                   m_sSceneLayout;
};

#endif // SCENEDATA_H
