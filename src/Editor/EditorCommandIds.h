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
            eChangeVersion,
            eChangeFont,
            eChangeDefaultLayout,
            eChangeMetronomeToyCommand,

            eAddResource,
            eChangeCurrentResource,
            eChangeFilter,
            eChangeResourceData,
            eChangeSource,
            eChangeTitleCard,
            eRemoveResource,
            eAddTag,
            eChangeTag,
            eRemoveTag,

            eChangeOpenedScript,
            eToggleEosCommand,
            eInsertEosCommand,
            eRemoveEosCommand,
            eUpdateEosCommand,

            eAddNodeItem,
            eMoveNodeItem,
            eChangeNodeItem,
            eRemoveConnecectionItem,

            eAddSequence,
            eChangeSequenceProperties,
            eChangeOpenedSequence,
            eAddRemoveSequenceLayer,
            eChangeSequenceLayerProperties,
            eAddOrRemoveSequenceElement,
            eChangeSequenceElementProperties
            )

#endif // EDITORCOMMANDIDS_H
