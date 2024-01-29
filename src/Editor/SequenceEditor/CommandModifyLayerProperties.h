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

private:
  void DoUnoRedo(const std::shared_ptr<SSequenceLayer>& spLayer);

  QPointer<CTimelineWidget>             m_pParent;
  qint32                                m_iIndex;
  std::shared_ptr<SSequenceLayer>       m_spLayerOld;
  std::shared_ptr<SSequenceLayer>       m_spLayerNew;
};

#endif // CCOMMANDMODIFYLAYERPROPERTIES_H
