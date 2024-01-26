#include "ZoomComboBox.h"

#include <QLineEdit>

CZoomComboBox::CZoomComboBox(QWidget* pParent) :
  QComboBox{pParent},
  m_viSteps({
         20,
         50,
         70,
         100,
         150,
         200
    })
{
  setProperty("styleSmall", true);
  setEditable(true);

  UpdateZoomComboBox(100);

  connect(this, qOverload<qint32>(&QComboBox::currentIndexChanged),
          this, &CZoomComboBox::ZoomIndexChanged);
  connect(this->lineEdit(), &QLineEdit::editingFinished,
          this, &CZoomComboBox::ZoomLineEditingFinished);
}
CZoomComboBox::~CZoomComboBox() = default;

//----------------------------------------------------------------------------------------
//
void CZoomComboBox::SetSteps(const std::vector<qint32>& viSteps)
{
  if (m_viSteps != viSteps)
  {
    m_viSteps = viSteps;
    UpdateZoomComboBox(m_iZoom);
  }
}

//----------------------------------------------------------------------------------------
//
void CZoomComboBox::UpdateZoomComboBox(qint32 iCurrentZoom)
{
  bool bWereSignalsBlocked = signalsBlocked();
  blockSignals(true);

  clear();
  setEditable(true);

  std::vector<qint32> viItems = m_viSteps;

  auto it = std::find(viItems.begin(), viItems.end(), iCurrentZoom);
  if (viItems.end() == it)
  {
    viItems.push_back(iCurrentZoom);
  }
  std::sort(viItems.begin(), viItems.end());

  for (const auto& iValue : viItems)
  {
    addItem(QString::number(iValue) + "%", iValue);
  }
  setCurrentIndex(findData(iCurrentZoom));

  lineEdit()->blockSignals(true);
  lineEdit()->setText(QString::number(iCurrentZoom) + "%");
  lineEdit()->blockSignals(false);
  blockSignals(bWereSignalsBlocked);
}

//----------------------------------------------------------------------------------------
//
qint32 CZoomComboBox::Zoom() const
{
  return m_iZoom;
}

//----------------------------------------------------------------------------------------
//
void CZoomComboBox::ZoomIndexChanged(qint32)
{
  bool bOk = false;
  qint32 iZoom = currentData().toInt(&bOk);
  if (bOk)
  {
    m_iZoom = iZoom;
    emit SignalZoomChanged(iZoom);
  }
}

//----------------------------------------------------------------------------------------
//
void CZoomComboBox::ZoomLineEditingFinished()
{
  bool bOk = false;
  qint32 iZoom = lineEdit()->text().replace("%", "").toInt(&bOk);
  if (bOk)
  {
    m_iZoom = iZoom;
    emit SignalZoomChanged(iZoom);
  }
}
