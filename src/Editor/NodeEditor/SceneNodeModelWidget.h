#ifndef SCENENODEMODELWIDGET_H
#define SCENENODEMODELWIDGET_H

#include "IUndoStackAwareModel.h"

#include "Systems/Project.h"

#include <QWidget>
#include <memory>

class QAbstractItemModel;
namespace Ui {
  class CSceneNodeModelWidget;
}

class CSceneNodeModelWidget : public QWidget, public IUndoStackAwareModel
{
  Q_OBJECT

public:
  explicit CSceneNodeModelWidget(QWidget* pParent = nullptr);
  ~CSceneNodeModelWidget();

  void SetName(const QString& sName);
  void SetProject(const tspProject& spProject);
  void SetScript(const QString& sName);
  void SetLayout(const QString& sName);
  void SetResourceItemModel(QAbstractItemModel* pModel);

  void OnLayoutAdded(const QString& sName);
  void OnLayoutRenamed(const QString& sOldName, const QString& sName);
  void OnLayoutRemoved(const QString& sName);

  void OnScriptAdded(const QString& sName);
  void OnScriptRenamed(const QString& sOldName, const QString& sName);
  void OnScriptRemoved(const QString& sName);

signals:
  void SignalTitleResourceChanged(const QString& sOld, const QString& sNew);
  void SignalAddScriptFileClicked(const QString&);
  void SignalAddLayoutFileClicked(const QString&);
  void SignalNameChanged(const QString& sName);
  void SignalScriptChanged(const QString& sName);
  void SignalLayoutChanged(const QString& sName);

protected slots:
  void on_FileIcon_SignalResourcePicked(const QString& sOld, const QString& sNew);
  void on_AddScriptFile_clicked();
  void on_AddLayoutFile_clicked();
  void on_pSceneNameLineEdit_editingFinished();
  void on_pScriptComboBox_currentIndexChanged(qint32 iIdx);
  void on_pLayoutComboBox_currentIndexChanged(qint32 iIdx);

private:
  std::unique_ptr<Ui::CSceneNodeModelWidget> m_spUi;
};

#endif // SCENENODEMODELWIDGET_H
