#ifndef CCOMMANDMODIFYLAYERS_H
#define CCOMMANDMODIFYLAYERS_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QUndoCommand>

#include <functional>

class CTimelineWidget;

class CCommandAddOrRemoveSequenceLayer : public QUndoCommand
{
public:
  CCommandAddOrRemoveSequenceLayer(CTimelineWidget* pParent,
                                   qint32 iIndex,
                                   const tspSequenceLayer& spLayer,
                                   std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> fnDo,
                                   std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> fnUndo,
                                   const QString& sOperation);
  ~CCommandAddOrRemoveSequenceLayer();

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  QPointer<CTimelineWidget>                                          m_pParent;
  std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> m_fnDo;
  std::function<void(qint32, const tspSequenceLayer&, tspSequence&)> m_fnUndo;
  tspSequenceLayer                                                   m_spLayer;
  qint32                                                             m_iIndex;
};

#endif // CCOMMANDMODIFYLAYERS_H
