#ifndef ENUMS_H
#define ENUMS_H

#include <QtGlobal>
#include <enum.h>

BETTER_ENUM(EAppState, qint32,
            eMainScreen = 0,
            eSceneScreen = 1,
            eSettingsScreen = 2,
            eEditorScreen = 3,
            eCreditsScreen = 4);

BETTER_ENUM(ECoreSystems, qint32,
            eDatabaseManager = 0,
            eScriptRunner = 1);

BETTER_ENUM(ELoadState, qint32,
            eUnstarted = 0,
            eLoading   = 1,
            eFinished  = 2,
            eError     = 3);

#endif // ENUMS_H
