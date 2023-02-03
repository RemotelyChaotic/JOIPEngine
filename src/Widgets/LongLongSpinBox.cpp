#include "LongLongSpinBox.h"
#include <QLineEdit>

CLongLongSpinBox::CLongLongSpinBox(QWidget* pParent) :
  QAbstractSpinBox{pParent}
{
  connect(lineEdit(), &QLineEdit::textEdited, this, &CLongLongSpinBox::SlotEditFinished);
  lineEdit()->setText(textFromValue(m_iValue));
}
CLongLongSpinBox::~CLongLongSpinBox()
{
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::fixup(QString& input) const
{
  QAbstractSpinBox::fixup(input);
}

//----------------------------------------------------------------------------------------
//
qint64 CLongLongSpinBox::minimum() const
{
  return m_iMin;
}

//----------------------------------------------------------------------------------------
//
qint64 CLongLongSpinBox::maximum() const
{
  return m_iMax;
}

//----------------------------------------------------------------------------------------
//
qint64 CLongLongSpinBox::value() const
{
  return m_iValue;
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::stepBy(int steps)
{
  qint64 iNewValue = m_iValue;
  if (steps < 0 && iNewValue + steps > iNewValue)
  {
    iNewValue = std::numeric_limits<qlonglong>::min();
  }
  else if (steps > 0 && iNewValue + steps < iNewValue)
  {
    iNewValue = std::numeric_limits<qlonglong>::max();
  }
  else
  {
    iNewValue += steps;
  }

  setValue(iNewValue);
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::setMinimum(qint64 iMin)
{
  if (iMin != m_iMin)
  {
    m_iMin = iMin;
    emit minimumChanged(iMin);
  }
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::setMaximum(qint64 iMax)
{
  if (iMax != m_iMax)
  {
    m_iMax = iMax;
    emit minimumChanged(iMax);
  }
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::setRange(qint64 min, qint64 max)
{
  setMinimum(min);
  setMaximum(max);
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::setValue(qint64 iValue)
{
  if (m_iValue != iValue)
  {
    lineEdit()->setText(textFromValue(iValue));
    m_iValue = iValue;
    emit valueChanged(iValue);
  }
}

//----------------------------------------------------------------------------------------
//
qlonglong CLongLongSpinBox::valueFromText(const QString& sText) const
{
  return sText.toLongLong();
}

//----------------------------------------------------------------------------------------
//
QString CLongLongSpinBox::textFromValue(qint64 iVal) const
{
  return QString::number(iVal);
}

//----------------------------------------------------------------------------------------
//
QAbstractSpinBox::StepEnabled CLongLongSpinBox::stepEnabled() const
{
  return StepUpEnabled | StepDownEnabled;
}

//----------------------------------------------------------------------------------------
//
QValidator::State CLongLongSpinBox::validate(QString& sInput, int&) const
{
  bool bOk;
  qlonglong val = sInput.toLongLong(&bOk);
  if (!bOk) { return QValidator::Invalid; }

  if (val < m_iMin || val > m_iMax)
  {
    return QValidator::Invalid;
  }

  return QValidator::Acceptable;
}

//----------------------------------------------------------------------------------------
//
void CLongLongSpinBox::SlotEditFinished()
{
  QString sInput = lineEdit()->text();
  int pos = 0;
  if (QValidator::Acceptable == validate(sInput, pos))
  {
    setValue(valueFromText(sInput));
  }
  else
  {
    lineEdit()->setText(textFromValue(m_iValue));
  }
}
