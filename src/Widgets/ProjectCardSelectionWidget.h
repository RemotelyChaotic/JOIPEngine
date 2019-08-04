#ifndef PROJECTCARDSELECTIONWIDGET_H
#define PROJECTCARDSELECTIONWIDGET_H

#include <QWidget>
#include <memory>

class CDatabaseManager;
namespace Ui {
  class CProjectCardSelectionWidget;
}
class QMovie;

class CProjectCardSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CProjectCardSelectionWidget(QWidget* pParent = nullptr);
  ~CProjectCardSelectionWidget();

  void Initialize();
  void LoadProjects();
  void UnloadProjects();
  qint32 SelectedId() { return m_iSelectedProjectId; }

private slots:
  void SlotCardClicked();

private:
  std::unique_ptr<Ui::CProjectCardSelectionWidget> m_spUi;
  std::shared_ptr<QMovie>                          m_spSpinner;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  qint32                                           m_iSelectedProjectId;
  bool                                             m_bInitialized;
};

#endif // PROJECTCARDSELECTIONWIDGET_H
