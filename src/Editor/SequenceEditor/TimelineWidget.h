#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "Systems/Sequence/Sequence.h"

#include <QPointer>
#include <QScrollArea>
#include <QUndoStack>

#include <functional>
#include <memory>

class CResourceTreeItemModel;
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
  Q_PROPERTY(QColor gridColor READ GridColor WRITE SetGridColor)
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
  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;
  void SetSelectionColor(const QColor& col);
  const QColor& SelectionColor() const;

  void AddNewLayer();
  void AddNewElement(const QString& sId, qint32 iLayer, qint64 iTimestamp);
  void Clear();
  qint32 IndexOf(CTimelineWidgetLayer* pLayer) const;
  CTimelineWidgetLayer* Layer(qint32 iIndex) const;
  qint32 LayerCount() const;
  void RemoveSelectedLayer();
  void RemoveSelectedElement();
  qint32 SelectedIndex() const;
  qint64 SelectedTimeStamp() const;
  void SetResourceModel(QPointer<CResourceTreeItemModel> pEditorModel);
  void SetSequence(const tspSequence& spSeq);
  void SetUndoStack(QPointer<QUndoStack> pUndo);
  void Update();

  tspSequence Sequence() const;

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

public slots:
  void SlotUpdateSequenceProperties();
  void SlotOpenInsertContextMenuRequested(qint32 iLayerIdx, qint64 iTimestamp, QPoint globalPos);

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
  void SlotOpenInsertContextMenuAt(QPoint p, qint64 iCursorTime);
  void SlotScrollbarValueChanged();
  void SlotSelectionColorChanged();
  void SlotTimeGridChanged(qint64 iGrid);
  void SlotZoomChanged(qint32 iZoom);

private:
  QWidget* CreateLayerWidget(const tspSequenceLayer& spLayer) const;
  void ForAllLayers(std::function<void(CTimelineWidgetLayer*,qint32)> fn);
  QSize HeadersSize() const;
  QString OpenInsertContextMenuAt(const QPoint& currentAddPoint, const QPoint& createPoint,
                                  const QString& sLayerType);
  bool IsChildOfLayer(QWidget* pLayer) const;
  void Resize(QSize newSize);
  void SetCurrentWindow();
  void SetSelectedTime();
  void UpdateTimeSelectionCursor();

  std::unique_ptr<Ui::CTimelineWidget> m_spUi;
  tspSequence                          m_spCurrentSequence;
  QPointer<CResourceTreeItemModel>     m_pEditorModel;
  QPointer<QUndoStack>                 m_pUndoStack;
  QPointer<CTimelineWidgetOverlay>     m_pOverlay;
  QPointer<CTimelineWidgetControls>    m_pControls;
  QPointer<QScrollBar>                 m_pCustomScrollbar;
  QColor                               m_alternateBgColor;
  QColor                               m_selectionColor;
  qint32                               m_iSelectedIndex = -1;
};

#endif // TIMELINEWIDGET_H
