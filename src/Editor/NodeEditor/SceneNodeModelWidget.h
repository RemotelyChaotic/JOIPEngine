#ifndef SCENENODEMODELWIDGET_H
#define SCENENODEMODELWIDGET_H

#include <QWidget>
#include <memory>

namespace Ui {
  class CSceneNodeModelWidget;
}

class CSceneNodeModelWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CSceneNodeModelWidget(QWidget* pParent = nullptr);
  ~CSceneNodeModelWidget();

  void SetName(const QString& sName);
  void SetScriptButtonEnabled(bool bEnabled);

signals:
  void SignalAddScriptFileClicked();
  void SignalNameChanged(const QString& sName);

protected slots:
  void on_AddScriptFile_clicked();
  void on_pSceneNameLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CSceneNodeModelWidget> m_spUi;
};

#endif // SCENENODEMODELWIDGET_H
