#ifndef EDITORCOMMANDIDS_H
#define EDITORCOMMANDIDS_H

#include <enum.h>
#include <QMetaType>
#include <QtGlobal>

BETTER_ENUM(EEditorCommandId, qint32,
            eNone               = -1,
            eChangeEmitterCount = 0,
            eAddFetishes,
            eRemoveFetishes,
            eChangeProjectName,
            eChangeVersion);

#endif // EDITORCOMMANDIDS_H
