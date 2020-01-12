#ifndef PROJECTCARDSELECTIONWIDGET_H
#define PROJECTCARDSELECTIONWIDGET_H

#include <QPointer>
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
  Q_PROPERTY(QColor selectionColor READ SelectionColor WRITE SetSelectionColor)

public:
  explicit CProjectCardSelectionWidget(QWidget* pParent = nullptr);
  ~CProjectCardSelectionWidget() override;

  void Initialize();
  void LoadProjects();
  void UnloadProjects();
  qint32 SelectedId() { return m_iSelectedProjectId; }

  void SetSelectionColor(const QColor& color);
  const QColor& SelectionColor();

protected slots:
  void SlotCardClicked();

private:
  void AddDropShadow(QWidget* pWidget, const QColor& color);

  std::unique_ptr<Ui::CProjectCardSelectionWidget> m_spUi;
  std::shared_ptr<QMovie>                          m_spSpinner;
  std::weak_ptr<CDatabaseManager>                  m_wpDbManager;
  QPointer<QWidget>                                m_pLastSelectedWidget;
  QColor                                           m_selectionColor;
  qint32                                           m_iSelectedProjectId;
  bool                                             m_bInitialized;
};

#endif // PROJECTCARDSELECTIONWIDGET_H
