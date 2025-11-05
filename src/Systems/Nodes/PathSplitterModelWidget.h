#ifndef PATHSPLITTERMODELWIDGET_H
#define PATHSPLITTERMODELWIDGET_H

#include "Systems/Database/Project.h"

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

  void SetCustomLayout(bool bEnabled, const QString& sName);
  void SetProject(const tspProject& spProject);
  void SetTransitionType(qint32 iType);
  void SetTransitionLabel(PortIndex index, const QString& sLabelValue);

  void OnLayoutAdded(const QString& sName);
  void OnLayoutRenamed(const QString& sOldName, const QString& sName);
  void OnLayoutRemoved(const QString& sName);

signals:
  void SignalTransitionTypeChanged(qint32 iType);
  void SignalTransitionLabelChanged(PortIndex index, const QString& sLabelValue);
  void SignalCustomTransitionChanged(bool bEnabled, const QString& sResource);
  void SignalAddLayoutFileClicked(const QString& sCustomInitContent);

protected slots:
  void on_pRandomRadio_clicked(bool bChecked);
  void on_pButtonLabelsGroupBox_clicked(bool bChecked);
  void on_pLabel1_editingFinished();
  void on_pLabel2_editingFinished();
  void on_pLabel3_editingFinished();
  void on_pLabel4_editingFinished();
  void on_pCustomTransitionCheckBox_clicked(bool bChecked);
  void on_pLayoutComboBox_currentIndexChanged(qint32 iIdx);
  void on_AddLayoutFile_clicked();

private:
  std::unique_ptr<Ui::CPathSplitterModelWidget> m_spUi;
};

#endif // PATHSPLITTERMODELWIDGET_H
