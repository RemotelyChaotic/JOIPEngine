#ifndef ENUMS_H
#define ENUMS_H

#include <QtGlobal>
#include <enum.h>

BETTER_ENUM(EAppState, qint32,
            eMainScreen = 0,
            eSceneSelectionScreen = 1,
            eSettingsScreen = 2,
            eSceneScreen = 3,
            eEditorScreen = 4);

BETTER_ENUM(ECoreSystems, qint32,
            eDatabaseManager = 0);

#endif // ENUMS_H
