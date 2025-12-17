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
            eChangeCanStartFomAnyScene,
            eAddAchievement,
            eChangeAchievement,
            eRemoveAchievement,
            eChangePluginPath,

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

            eAddDialogueNode,
            eRemoveDialogueNode,
            eAddNewDialogueFile,
            eChangeDialoguePropertyViaGui,
            eChangeDialogueParams,
            eAddDialogueTags,
            eRemoveDialogueTags
            )

#endif // EDITORCOMMANDIDS_H
