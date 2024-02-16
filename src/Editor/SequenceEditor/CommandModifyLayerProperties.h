#ifndef CCOMMANDMODIFYLAYERPROPERTIES_H
#define CCOMMANDMODIFYLAYERPROPERTIES_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QUndoCommand>

#include <memory>

class CTimelineWidget;

class CCommandModifyLayerProperties : public QUndoCommand
{
public:
  CCommandModifyLayerProperties(QPointer<CTimelineWidget> pParent,
                                const std::shared_ptr<SSequenceLayer>& layerOld,
                                const std::shared_ptr<SSequenceLayer>& layerNew,
                                qint32 iIndex,
                                const QString& sCommandDescr);
  ~CCommandModifyLayerProperties();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void DoUnoRedo(const std::shared_ptr<SSequenceLayer>& spLayer);

  QPointer<CTimelineWidget>             m_pParent;
  qint32                                m_iIndex;
  std::shared_ptr<SSequenceLayer>       m_spLayerOld;
  std::shared_ptr<SSequenceLayer>       m_spLayerNew;
};

//----------------------------------------------------------------------------------------
//
class CCommandAddOrRemoveInstruction : public CCommandModifyLayerProperties
{
public:
  CCommandAddOrRemoveInstruction(QPointer<CTimelineWidget> pParent,
                                 const std::shared_ptr<SSequenceLayer>& layerOld,
                                 const std::shared_ptr<SSequenceLayer>& layerNew,
                                 qint32 iIndex,
                                 const QString& sCommandDescr);
  ~CCommandAddOrRemoveInstruction();

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;
};

//----------------------------------------------------------------------------------------
//
class CCommandChangeInstructionParameters : public CCommandModifyLayerProperties
{
public:
  CCommandChangeInstructionParameters(QPointer<CTimelineWidget> pParent,
                                      const std::shared_ptr<SSequenceLayer>& layerOld,
                                      const std::shared_ptr<SSequenceLayer>& layerNew,
                                      qint32 iIndex,
                                      sequence::tTimePos iInstruction,
                                      const QString& sCommandDescr);
  ~CCommandChangeInstructionParameters();

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  sequence::tTimePos m_iInstruction;
};

#endif // CCOMMANDMODIFYLAYERPROPERTIES_H
