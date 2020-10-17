#ifndef EDITORWIDGETTYPES_H
#define EDITORWIDGETTYPES_H

#include "enum.h"
#include <QMetaType>
#include <QtGlobal>

BETTER_ENUM(EEditorWidget, qint32,
            eResourceWidget = 0,
            eResourceDisplay = 1,
            eProjectSettings = 2,
            eSceneNodeWidget = 3,
            eSceneCodeEditorWidget = 4);

#endif // EDITORWIDGETTYPES_H
