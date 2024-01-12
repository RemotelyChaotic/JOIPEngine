#ifndef CTIMELINEWIDGETLAYER_H
#define CTIMELINEWIDGETLAYER_H

#include "Systems/Sequence/Sequence.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QScrollArea>
#include <QWidget>

class CTimelineWidgetLayer : public QWidget
{
  Q_OBJECT
public:
  explicit CTimelineWidgetLayer(const tspSequenceLayer& spLayer, QWidget* pParent = nullptr);
  ~CTimelineWidgetLayer() override;

  void SetLayer(const tspSequenceLayer& spLayer);

  QString Name() const;
  QString LayerType() const;

signals:

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  tspSequenceLayer      m_spLayer;
  QPointer<QScrollArea> m_pTimeLineContent;
  QPointer<QLineEdit>   m_pNameLineEdit;
  QPointer<QComboBox>   m_pLayerTypeCombo;
};

#endif // CTIMELINEWIDGETLAYER_H
