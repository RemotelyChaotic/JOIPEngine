#ifndef CTIMELINEWIDGETLAYER_H
#define CTIMELINEWIDGETLAYER_H

#include "Systems/Sequence/Sequence.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QUndoStack>
#include <QWidget>

class CTimelineWidget;
class CTimeLinewidgetLayerShadow;

class CTimelineWidgetLayer : public QFrame
{
  Q_OBJECT
public:
  explicit CTimelineWidgetLayer(const tspSequenceLayer& spLayer, CTimelineWidget* pParent,
                                QWidget* pWidgetParent);
  ~CTimelineWidgetLayer() override;

  void SetLayer(const tspSequenceLayer& spLayer);
  void SetHighlight(QColor col, QColor alternateCol);
  void SetUndoStack(QPointer<QUndoStack> pUndo);

  QString Name() const;
  tspSequenceLayer Layer() const;
  QString LayerType() const;
  QSize HeaderSize() const;

  void UpdateUi();

signals:
  void SignalUserStartedDrag();
  void SignalSelected();

protected:
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvt) override;

private slots:
  void SlotLabelChanged();
  void SlotTypeChanged();

private:
  tspSequenceLayer      m_spLayer;
  QPointer<CTimelineWidget> m_pParent;
  QPointer<CTimeLinewidgetLayerShadow> m_pDropShadow;
  QPointer<QScrollArea> m_pTimeLineContent;
  QPointer<QLineEdit>   m_pNameLineEdit;
  QPointer<QComboBox>   m_pLayerTypeCombo;
  QPointer<QWidget>     m_pHeader;
  QPointer<QUndoStack>  m_pUndoStack;
  QPoint                m_dragDistance;
  QPoint                m_dragOrigin;
  QColor                m_alternateBgCol;
  bool                  m_bMousePotentionallyStartedDrag = false;
};

#endif // CTIMELINEWIDGETLAYER_H
