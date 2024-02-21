#ifndef CTIMELINEWIDGETLAYER_H
#define CTIMELINEWIDGETLAYER_H

#include "Systems/Sequence/Sequence.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QUndoStack>
#include <QWidget>

class CResourceTreeItemModel;
class CTimelineSeqeunceInstructionConfigOverlay;
class CTimelineWidget;
class CTimelineWidgetLayerBackground;
class CTimeLinewidgetLayerShadow;

class CTimelineWidgetLayer : public QFrame
{
  Q_OBJECT
  Q_PROPERTY(QColor selectionColor READ SelectionColor WRITE SetSelectionColor)
  Q_PROPERTY(QColor timelineBackgroundColor READ TimelineBackgroundColor WRITE SetTimelineBackgroundColor)

public:
  explicit CTimelineWidgetLayer(const tspSequenceLayer& spLayer, CTimelineWidget* pParent,
                                QWidget* pWidgetParent, QPointer<CResourceTreeItemModel> pItemModel);
  ~CTimelineWidgetLayer() override;

  void SetGridColor(const QColor& col);
  const QColor& GridColor() const;
  void SetOutOfRangeColor(const QColor& col);
  const QColor& OutOfRangeColor() const;
  void SetSelectionColor(const QColor& col);
  const QColor& SelectionColor() const;
  void SetTimelineBackgroundColor(const QColor& col);
  const QColor& TimelineBackgroundColor() const;

  void AddNewElement(const QString& sId, qint64 iTimestamp);
  void ClearSelection();
  void CloseConfigOverlay();
  void RemoveSelectedElement();
  void SetLayer(const tspSequenceLayer& spLayer);
  void SetHighlight(QColor col, QColor alternateCol);
  void SetTimeGrid(qint64 iGrid);
  void SetUndoStack(QPointer<QUndoStack> pUndo);

  QString Name() const;
  tspSequenceLayer Layer() const;
  QString LayerType() const;
  QSize HeaderSize() const;
  QPointer<CResourceTreeItemModel> ResourceItemModel() const;

  void UpdateUi();
  void SetCurrentWindow(qint64 iStartMs, qint64 iPageLengthMs);
  void SetTimeMaximum(qint64 iTimeMs);

signals:
  void SignalOpenInsertContextMenuAt(QPoint p, qint64 iCursorTime);
  void SignalUserStartedDrag();
  void SignalSelected();

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvt) override;
  void resizeEvent(QResizeEvent* pEvt) override;

private slots:
  void SlotInstructionChanged();
  void SlotLabelChanged();
  void SlotSelectedInstruction(qint64 iInstr);
  void SlotTypeChanged();

private:
  tspSequenceLayer      m_spLayer;
  QPointer<CResourceTreeItemModel> m_pEditorModel;
  QPointer<CTimelineWidget> m_pParent;
  QPointer<CTimeLinewidgetLayerShadow> m_pDropShadow;
  QPointer<CTimelineWidgetLayerBackground> m_pTimeLineContent;
  QPointer<QLineEdit>   m_pNameLineEdit;
  QPointer<QComboBox>   m_pLayerTypeCombo;
  QPointer<QWidget>     m_pHeader;
  QPointer<QUndoStack>  m_pUndoStack;
  QPointer<CTimelineSeqeunceInstructionConfigOverlay> m_pConfigOverlay;
  QPoint                m_dragDistance;
  QPoint                m_dragOrigin;
  QColor                m_alternateBgCol;
  QColor                m_selectionColor;
  bool                  m_bMousePotentionallyStartedDrag = false;
};

#endif // CTIMELINEWIDGETLAYER_H
