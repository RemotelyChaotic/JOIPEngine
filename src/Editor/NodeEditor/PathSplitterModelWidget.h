#ifndef PATHSPLITTERMODELWIDGET_H
#define PATHSPLITTERMODELWIDGET_H

#include <nodes/Node>
#include <QWidget>
#include <array>
#include <memory>

namespace Ui {
  class CPathSplitterModelWidget;
}

using QtNodes::PortIndex;

class CPathSplitterModelWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CPathSplitterModelWidget(QWidget* pParent = nullptr);
  ~CPathSplitterModelWidget();

  void SetTransitionType(qint32 iType);
  void SetTransitionLabel(PortIndex index, const QString& sLabelValue);

signals:
  void SignalTransitionTypeChanged(qint32 iType);
  void SignalTransitionLabelChanged(PortIndex index, const QString& sLabelValue);

protected slots:
  void on_pRandomRadio_clicked(bool bChecked);
  void on_pSelectionRadio_clicked(bool bChecked);
  void on_pLabel1_editingFinished();
  void on_pLabel2_editingFinished();
  void on_pLabel3_editingFinished();
  void on_pLabel4_editingFinished();

private:
  std::unique_ptr<Ui::CPathSplitterModelWidget> m_spUi;
};

#endif // PATHSPLITTERMODELWIDGET_H
