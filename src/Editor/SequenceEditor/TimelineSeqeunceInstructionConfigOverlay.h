#ifndef CTIMELINESEQEUNCEINSTRUCTIONCONFIGOVERLAY_H
#define CTIMELINESEQEUNCEINSTRUCTIONCONFIGOVERLAY_H

#include "Systems/Sequence/Sequence.h"
#include "Widgets/OverlayBase.h"

#include <QPointer>
#include <memory>

class CResourceTreeItemModel;
class CTimelineWidgetLayer;
class CTimelineSeqeunceInstructionConfigOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionConfigOverlay(CTimelineWidgetLayer* pParent);
  ~CTimelineSeqeunceInstructionConfigOverlay() override;

  void Show(qint64 iInstr, const std::shared_ptr<SSequenceInstruction>& spInstr);

  std::shared_ptr<SSequenceInstruction> CurrentInstructionParameters() const;
  bool IsForcedOpen() const;

public slots:
  void Climb() override;
  void Hide() override;
  void Resize() override;

signals:
  void SignalCurrentItemChanged();

private:
  void ClearLayout();

  std::shared_ptr<SSequenceInstruction>  m_spInstr;
  QPointer<CTimelineWidgetLayer>         m_pParent;
  qint64                                 m_iInstrId = -1;
};

#endif // CTIMELINESEQEUNCEINSTRUCTIONCONFIGOVERLAY_H
