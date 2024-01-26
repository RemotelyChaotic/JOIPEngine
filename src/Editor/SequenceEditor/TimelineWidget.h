#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QScrollArea>
#include <QUndoStack>
#include <memory>

class CTimelineWidgetControls;
class CTimelineWidgetLayer;
class CTimelineWidgetOverlay;
namespace Ui {
  class CTimelineWidget;
}

class CTimelineWidget : public QScrollArea
{
  Q_OBJECT
  Q_PROPERTY(QColor alternateBackgroundColor READ AlternateBackgroundColor WRITE SetAlternateBackgroundColor NOTIFY SignalAlternateBackgroundColorChanged)
  Q_PROPERTY(QColor dropIndicationColor READ DropIndicationColor WRITE SetDropIndicationColor)
  Q_PROPERTY(QColor outOfRangeColor READ OutOfRangeColor WRITE SetOutOfRangeColor)
  Q_PROPERTY(QColor selectionColor READ SelectionColor WRITE SetSelectionColor NOTIFY SignalSelectionColorChanged)

public:
  explicit CTimelineWidget(QWidget* pParent = nullptr);
  ~CTimelineWidget();

  //shadow because qt is dumb sometimes
  void setWidget(QWidget* pWidget);

  void SetAlternateBackgroundColor(const QColor& col);
  const QColor& AlternateBackgroundColor() const;
  void SetDropIndicationColor(const QColor& col);
  const QColor& DropIndicationColor() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;
  void SetSelectionColor(const QColor& col);
  const QColor& SelectionColor() const;

  void AddNewLayer();
  void AddNewElement(const QString& sId);
  void Clear();
  qint32 IndexOf(CTimelineWidgetLayer* pLayer) const;
  CTimelineWidgetLayer* Layer(qint32 iIndex) const;
  qint32 LayerCount() const;
  void RemoveSelectedLayer();
  qint32 SelectedIndex() const;
  void SetSequence(const tspSequence& spSeq);
  void SetUndoStack(QPointer<QUndoStack> pUndo);
  void Update();

  tspSequence Sequence() const;

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

public slots:
  void SlotUpdateSequenceProperties();

signals:
  void SignalAlternateBackgroundColorChanged();
  void SignalContentsChanged();
  void SignalSelectionColorChanged();

protected:
  void AddLayerTo(qint32 index, const tspSequenceLayer& layer,
                  tspSequence& spCurrentSequence);
  void RemoveLayerFrom(qint32 index, tspSequence& spCurrentSequence);

  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void dragEnterEvent(QDragEnterEvent* pEvent) override;
  void dragMoveEvent(QDragMoveEvent* pEvent) override;
  void dropEvent(QDropEvent* pEvent) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void resizeEvent(QResizeEvent* pEvt) override;
  void wheelEvent(QWheelEvent* pEVt) override;

private slots:
  void SlotUserStartedDrag();
  void SlotLayerSelected();
  void SlotLayersInserted();
  void SlotScrollbarValueChanged();
  void SlotSelectionColorChanged();
  void SlotZoomChanged(qint32 iZoom);

private:
  QWidget* CreateLayerWidget(const tspSequenceLayer& spLayer) const;
  QSize HeadersSize() const;
  bool IsChildOfLayer(QWidget* pLayer) const;
  void Resize(QSize newSize);

  std::unique_ptr<Ui::CTimelineWidget> m_spUi;
  tspSequence                          m_spCurrentSequence;
  QPointer<QUndoStack>                 m_pUndoStack;
  QPointer<CTimelineWidgetOverlay>     m_pOverlay;
  QPointer<CTimelineWidgetControls>    m_pControls;
  QPointer<QScrollBar>                 m_pCustomScrollbar;
  QColor                               m_alternateBgColor;
  QColor                               m_selectionColor;
  qint32                               m_iSelectedIndex = -1;
};

#endif // TIMELINEWIDGET_H
