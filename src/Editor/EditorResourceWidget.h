#ifndef EDITORRESOURCEWIDGET_H
#define EDITORRESOURCEWIDGET_H

#include <QWidget>
#include <memory>

class CDatabaseManager;
namespace Ui {
  class CEditorResourceWidget;
}
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorResourceWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CEditorResourceWidget(QWidget* pParent = nullptr);
  ~CEditorResourceWidget();

  void Initialize();
  void LoadProject(tspProject spCurrentProject);
  void UnloadProject();

protected slots:
  void on_pFilterLineEdit_editingFinished();
  void on_pAddButton_clicked();
  void on_pAddWebButton_clicked();
  void on_pRemoveButton_clicked();

private:
  std::unique_ptr<Ui::CEditorResourceWidget> m_spUi;
  tspProject                                 m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>            m_wpDbManager;
  bool                                       m_bInitialized;
};

#endif // EDITORRESOURCEWIDGET_H
