#ifndef EDITORCOMMANDIDS_H
#define EDITORCOMMANDIDS_H

#include <enum.h>
#include <QMetaType>
#include <QtGlobal>

BETTER_ENUM(EEditorCommandId, qint32,
            eNone = -1,

            eChangeTag = 0,

            eChangeEmitterCount,
            eAddFetishes,
            eRemoveFetishes,
            eChangeProjectName,
            eChangeVersion,
            eChangeFont,
            eChangeDefaultLayout,
            eChangeMetronomeToyCommand,
            eAddAchievement,
            eChangeAchievement,
            eRemoveAchievement,

            eAddResource,
            eChangeCurrentResource,
            eChangeFilter,
            eChangeResourceData,
            eChangeSource,
            eChangeTitleCard,
            eRemoveResource,
            eAddTag,
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
            eChangeSequenceElementProperties,

            eAddDialogNode,
            eRemoveDialogNode,
            eAddNewDialogFile,
            eChangeDialogPropertyViaGui,
            eChangeDialogParams,
            eAddDialogTags,
            eRemoveDialogTags
            )

#endif // EDITORCOMMANDIDS_H
