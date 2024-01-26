#ifndef CZOOMCOMBOBOX_H
#define CZOOMCOMBOBOX_H

#include <QComboBox>

class CZoomComboBox : public QComboBox
{
  Q_OBJECT
public:
  explicit CZoomComboBox(QWidget* pParent = nullptr);
  ~CZoomComboBox() override;

  void SetSteps(const std::vector<qint32>& viSteps);
  void UpdateZoomComboBox(qint32 iCurrentZoom);
  qint32 Zoom() const;

signals:
  void SignalZoomChanged(qint32 iZoom);

private slots:
  void ZoomIndexChanged(qint32);
  void ZoomLineEditingFinished();

private:
  qint32 m_iZoom = 100;
  std::vector<qint32> m_viSteps;
};

#endif // CZOOMCOMBOBOX_H
