#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QScrollArea>
#include <QUndoStack>
#include <memory>

namespace Ui {
  class CTimelineWidget;
}

class CTimelineWidget : public QScrollArea
{
  Q_OBJECT

public:
  explicit CTimelineWidget(QWidget* pParent = nullptr);
  ~CTimelineWidget();

  void AddNewLayer();
  void AddNewElement(const QString& sId);
  void Clear();
  void RemoveSelectedLayer();
  void SetSequence(const tspSequence& spSeq);
  void SetUndoStack(QPointer<QUndoStack> pUndo);
  void Update();

  tspSequence Sequence() const;

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

signals:
  void SignalContentsChanged();

protected:
  void AddLayerTo(qint32 index, const tspSequenceLayer& layer,
                  tspSequence& spCurrentSequence);
  void RemoveLayerFrom(qint32 index, tspSequence& spCurrentSequence);

  void paintEvent(QPaintEvent* pEvt) override;
  void resizeEvent(QResizeEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineWidget> m_spUi;
  tspSequence                          m_spCurrentSequence;
  QPointer<QUndoStack>                 m_pUndoStack;
  qint32                               m_iSelectedIndex = -1;
};

#endif // TIMELINEWIDGET_H
