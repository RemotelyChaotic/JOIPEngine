#include "ColorPicker.h"
#include "ui_ColorPicker.h"

CColorPicker::CColorPicker(QWidget *parent) :
  QWidget(parent),
  m_spUi(new Ui::CColorPicker),
  m_color(0, 0, 0, 255)
{
  m_spUi->setupUi(this);
}

CColorPicker::~CColorPicker()
{
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::SetColor(const QColor& color)
{
  if (m_color != color)
  {
    m_spUi->pSpinBoxR->blockSignals(true);
    m_spUi->pSpinBoxG->blockSignals(true);
    m_spUi->pSpinBoxB->blockSignals(true);
    m_spUi->pSpinBoxA->blockSignals(true);

    m_spUi->pSpinBoxR->setValue(color.red());
    m_spUi->pSpinBoxG->setValue(color.green());
    m_spUi->pSpinBoxB->setValue(color.blue());
    m_spUi->pSpinBoxA->setValue(color.alpha());

    m_spUi->pSpinBoxR->blockSignals(false);
    m_spUi->pSpinBoxG->blockSignals(false);
    m_spUi->pSpinBoxB->blockSignals(false);
    m_spUi->pSpinBoxA->blockSignals(false);

    m_color = color;
    UpdateWidgetDisplay();
    emit SignalColorChanged(m_color);
  }
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::SetAlphaVisible(bool bVisible)
{
  m_spUi->pSpinBoxA->setVisible(bVisible);
}

//----------------------------------------------------------------------------------------
//
bool CColorPicker::IsAlphaVisible()
{
  return m_spUi->pSpinBoxA->isVisible();
}

//----------------------------------------------------------------------------------------
//
QColor CColorPicker::Color() const
{
  return m_color;
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::on_pSpinBoxR_valueChanged(int i)
{
  if (i != m_color.red())
  {
    m_color.setRed(i);
    UpdateWidgetDisplay();
    emit SignalColorChanged(m_color);
  }
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::on_pSpinBoxG_valueChanged(int i)
{
  if (i != m_color.green())
  {
    m_color.setGreen(i);
    UpdateWidgetDisplay();
    emit SignalColorChanged(m_color);
  }
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::on_pSpinBoxB_valueChanged(int i)
{
  if (i != m_color.blue())
  {
    m_color.setBlue(i);
    UpdateWidgetDisplay();
    emit SignalColorChanged(m_color);
  }
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::on_pSpinBoxA_valueChanged(int i)
{
  if (i != m_color.alpha())
  {
    m_color.setAlpha(i);
    UpdateWidgetDisplay();
    emit SignalColorChanged(m_color);
  }
}

//----------------------------------------------------------------------------------------
//
void CColorPicker::UpdateWidgetDisplay()
{
  m_spUi->pColorDisplay->setStyleSheet(QString("QWidget {"
                                       "background-color: rgba(%1, %2, %3, %4);"
                                       "border 1px solid silver; }")
                                       .arg(QString::number(m_color.red()))
                                       .arg(QString::number(m_color.green()))
                                       .arg(QString::number(m_color.blue()))
                                       .arg(QString::number(m_color.alpha())));
}
