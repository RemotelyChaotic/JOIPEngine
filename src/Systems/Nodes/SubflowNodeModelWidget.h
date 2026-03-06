#ifndef SUBFLOWNODEMODELWIDGET_H
#define SUBFLOWNODEMODELWIDGET_H

#include "Systems/Database/Project.h"

#include <QWidget>

#include <memory>

namespace Ui {
  class CSubflowNodeModelWidget;
}

class CSubflowNodeModelWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSubflowNodeModelWidget(QWidget* pParent = nullptr);
  ~CSubflowNodeModelWidget();

  void SetFlow(const QString& sFlow);
  void SetProject(const tspProject& spProject);
  void SetNodeName(const QString& sFlow);

  void OnFlowAdded(const QString& sName);
  void OnFlowRenamed(const QString& sOldName, const QString& sName);
  void OnFlowRemoved(const QString& sName);

signals:
  void SignalAddNodeFileClicked();
  void SignalFlowChanged(const QString& sName);
  void SignalNameChanged(const QString& sName);

protected slots:
  void on_pNameLineEdit_editingFinished();
  void on_pFlowComboBox_currentIndexChanged(qint32 iIdx);
  void on_CreateNewFlow_clicked();

private:
  std::unique_ptr<Ui::CSubflowNodeModelWidget> m_spUi;
};

#endif // SUBFLOWNODEMODELWIDGET_H
