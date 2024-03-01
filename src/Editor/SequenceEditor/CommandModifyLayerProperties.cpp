#include "CommandModifyLayerProperties.h"
#include "TimelineWidget.h"
#include "TimelineWidgetLayer.h"

#include "Editor/EditorCommandIds.h"

CCommandModifyLayerProperties::CCommandModifyLayerProperties(QPointer<CTimelineWidget> pParent,
                                                             const std::shared_ptr<SSequenceLayer>& layerOld,
                                                             const std::shared_ptr<SSequenceLayer>& layerNew,
                                                             qint32 iIndex,
                                                             const QString& sCommandDescr) :
  QUndoCommand("Changed properties of layer: " + sCommandDescr, nullptr),
  m_pParent(pParent),
  m_iIndex(iIndex),
  m_spLayerOld(layerOld),
  m_spLayerNew(layerNew)
{
}
CCommandModifyLayerProperties::~CCommandModifyLayerProperties() = default;

//----------------------------------------------------------------------------------------
//
void CCommandModifyLayerProperties::undo()
{
  DoUnoRedo(m_spLayerOld);
}

//----------------------------------------------------------------------------------------
//
void CCommandModifyLayerProperties::redo()
{
  DoUnoRedo(m_spLayerNew);
}

//----------------------------------------------------------------------------------------
//
int CCommandModifyLayerProperties::id() const
{
  return EEditorCommandId::eChangeSequenceLayerProperties;
}

//----------------------------------------------------------------------------------------
//
bool CCommandModifyLayerProperties::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandModifyLayerProperties* pOtherCasted = dynamic_cast<const CCommandModifyLayerProperties*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (nullptr == m_spLayerNew) { return false; }

  m_spLayerNew = pOtherCasted->m_spLayerNew;

  setText(QString("Changed multiple properties of layer: ") + m_spLayerOld->m_sName);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CCommandModifyLayerProperties::DoUnoRedo(const std::shared_ptr<SSequenceLayer>& spLayer)
{
  if (nullptr != m_pParent)
  {
    if (auto pLayer = m_pParent->Layer(m_iIndex))
    {
      auto spLayerToMod = pLayer->Layer();
      spLayerToMod->m_sName = spLayer->m_sName;
      spLayerToMod->m_sLayerType = spLayer->m_sLayerType;
      spLayerToMod->m_vspInstructions.clear();
      for (const auto& [time, instr] : spLayer->m_vspInstructions)
      {
        spLayerToMod->m_vspInstructions.push_back({time, instr->Clone()});
      }
      pLayer->UpdateUi();
      emit pLayer->SignalContentsChanged();
    }
  }
}

//----------------------------------------------------------------------------------------
//
CCommandAddOrRemoveInstruction::CCommandAddOrRemoveInstruction(
    QPointer<CTimelineWidget> pParent,
    const std::shared_ptr<SSequenceLayer>& layerOld,
    const std::shared_ptr<SSequenceLayer>& layerNew,
    qint32 iIndex,
    const QString& sCommandDescr) :
  CCommandModifyLayerProperties(pParent, layerOld, layerNew, iIndex, QString())
{
  setText(QString("Layer %1: %2").arg(layerOld->m_sName).arg(sCommandDescr));
}
CCommandAddOrRemoveInstruction::~CCommandAddOrRemoveInstruction() = default;

//----------------------------------------------------------------------------------------
//
int CCommandAddOrRemoveInstruction::id() const
{
  return EEditorCommandId::eAddOrRemoveSequenceElement;
}

//----------------------------------------------------------------------------------------
//
bool CCommandAddOrRemoveInstruction::mergeWith(const QUndoCommand*)
{
  return false;
}

//----------------------------------------------------------------------------------------
//
CCommandChangeInstructionParameters::CCommandChangeInstructionParameters(
    QPointer<CTimelineWidget> pParent,
    const std::shared_ptr<SSequenceLayer>& layerOld,
    const std::shared_ptr<SSequenceLayer>& layerNew,
    qint32 iIndex,
    sequence::tTimePos iInstruction,
    const QString& sCommandDescr) :
  CCommandModifyLayerProperties(pParent, layerOld, layerNew, iIndex, QString()),
  m_iInstruction(iInstruction)
{
  setText(QString("Layer %1: %2").arg(layerOld->m_sName).arg(sCommandDescr));
}
CCommandChangeInstructionParameters::~CCommandChangeInstructionParameters() = default;

//----------------------------------------------------------------------------------------
//
int CCommandChangeInstructionParameters::id() const
{
  return EEditorCommandId::eChangeSequenceElementProperties;
}

//----------------------------------------------------------------------------------------
//
bool CCommandChangeInstructionParameters::mergeWith(const QUndoCommand* pOther)
{
  if (nullptr == pOther || id() != pOther->id())
  {
    return false;
  }

  const CCommandChangeInstructionParameters* pOtherCasted = dynamic_cast<const CCommandChangeInstructionParameters*>(pOther);
  if (nullptr == pOtherCasted) { return false; }
  if (nullptr == m_spLayerNew) { return false; }
  if (m_iInstruction != pOtherCasted->m_iInstruction) { return false; }

  m_spLayerNew = pOtherCasted->m_spLayerNew;
  return true;
}
