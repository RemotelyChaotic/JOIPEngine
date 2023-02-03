#ifndef CLONGLONGSPINBOX_H
#define CLONGLONGSPINBOX_H

#include <QAbstractSpinBox>
#include <limits>

class CLongLongSpinBox : public QAbstractSpinBox
{
  Q_OBJECT
  Q_DISABLE_COPY(CLongLongSpinBox)
  Q_PROPERTY(qint64 minimum READ minimum WRITE setMinimum NOTIFY minimumChanged)
  Q_PROPERTY(qint64 maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)
  Q_PROPERTY(qint64 value READ value WRITE setValue NOTIFY valueChanged)

public:
  explicit CLongLongSpinBox(QWidget* pParent = nullptr);
  ~CLongLongSpinBox() override;

  void fixup(QString &input) const override;
  qint64 minimum() const;
  qint64 maximum() const;
  qint64 value() const;

  void stepBy(int steps) override;
  void setMinimum(qint64 iMin);
  void setMaximum(qint64 iMax);
  void setRange(qint64 min, qint64 max);

public slots:
  void setValue(qint64 iValue);

signals:
  void minimumChanged(qint64 iMin);
  void maximumChanged(qint64 iMax);
  void valueChanged(qint64 iValue);

protected:
  virtual qlonglong valueFromText(const QString& sText) const;
  virtual QString textFromValue(qint64 iVal) const;
  QAbstractSpinBox::StepEnabled stepEnabled() const override;
  QValidator::State validate(QString& sInput, int& iPos) const override;

private slots:
  void SlotEditFinished();

private:
  qint64 m_iMin = 0;
  qint64 m_iMax = LLONG_MAX;
  qint64 m_iValue = 0;
};

#endif // CLONGLONGSPINBOX_H
