#ifndef CTIMELINEWIDGETLAYER_H
#define CTIMELINEWIDGETLAYER_H

#include "Systems/Sequence/Sequence.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QWidget>

class CTimeLinewidgetLayerShadow;

class CTimelineWidgetLayer : public QFrame
{
  Q_OBJECT
public:
  explicit CTimelineWidgetLayer(const tspSequenceLayer& spLayer, QWidget* pParent = nullptr);
  ~CTimelineWidgetLayer() override;

  void SetLayer(const tspSequenceLayer& spLayer);
  void SetHighlight(QColor col, QColor alternateCol);

  QString Name() const;
  tspSequenceLayer Layer() const;
  QString LayerType() const;

signals:
  void SignalUserStartedDrag();
  void SignalSelected();

protected:
  void mousePressEvent(QMouseEvent* pEvent) override;
  void mouseMoveEvent(QMouseEvent* pEvent) override;
  void mouseReleaseEvent(QMouseEvent* pEvent) override;
  void paintEvent(QPaintEvent* pEvt) override;

private:
  tspSequenceLayer      m_spLayer;
  QPointer<CTimeLinewidgetLayerShadow> m_pDropShadow;
  QPointer<QScrollArea> m_pTimeLineContent;
  QPointer<QLineEdit>   m_pNameLineEdit;
  QPointer<QComboBox>   m_pLayerTypeCombo;
  QPointer<QWidget>     m_pHeader;
  QPoint                m_dragDistance;
  QPoint                m_dragOrigin;
  QColor                m_alternateBgCol;
  bool                  m_bMousePotentionallyStartedDrag = false;
};

#endif // CTIMELINEWIDGETLAYER_H
