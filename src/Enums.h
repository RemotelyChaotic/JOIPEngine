#ifndef ENUMS_H
#define ENUMS_H

#include <QtGlobal>
#include <enum.h>
#include <limits>

BETTER_ENUM(EAppState, qint32,
            eMainScreen = 0,
            eSceneScreen = 1,
            eSettingsScreen = 2,
            eEditorScreen = 3,
            eCreditsScreen = 4);

BETTER_ENUM(ECoreSystems, qint32,
            eDatabaseManager = 0);

BETTER_ENUM(ELoadState, qint32,
            eUnstarted = 0,
            eLoading   = 1,
            eFinished  = 2,
            eError     = 3);

BETTER_ENUM(ETutorialState, qint32,
            eUnstarted = 0,
            eBeginTutorial,
            eSwitchRightPanelToProjectSettings,
            eFinished  = 63); // maximum value for Better-Enum

BETTER_ENUM(EAnchors, qint32,
            eCenter = 0,
            eLeft = 1,
            eRight = 2,
            eTop = 3,
            eBottom = 4,
            eTopLeft = 5,
            eTopRight = 6,
            eBottomLeft = 7,
            eBottomRight = 8);

#endif // ENUMS_H
