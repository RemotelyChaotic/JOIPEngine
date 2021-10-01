#ifndef CEDITORLAYOUTCLASSIC_H
#define CEDITORLAYOUTCLASSIC_H

#include "EditorLayoutBase.h"
#include "Editor/EditorWidgetRegistry.h"

class CDatabaseManager;
class CEditorActionBar;
namespace Ui {
  class CEditorLayoutClassic;
}
class QComboBox;
class QGroupBox;

class CEditorLayoutClassic : public CEditorLayoutBase
{
  Q_OBJECT
  friend class CMainScreenTutorialStateSwitchHandler;

public:
  explicit CEditorLayoutClassic(QWidget* pParent = nullptr);
  ~CEditorLayoutClassic() override;

  void ProjectLoaded(tspProject spCurrentProject, bool bModified) override;
  void ProjectUnloaded() override;

protected slots:
  void on_pLeftComboBox_currentIndexChanged(qint32 iIndex);
  void on_pRightComboBox_currentIndexChanged(qint32 iIndex);
  void SlotDisplayResource(const QString& sName);
  void SlotKeyBindingsChanged();

protected:
  void InitializeImpl() override;
  QPointer<QComboBox> LeftComboBox() const;
  QPointer<QGroupBox> LeftGroupBox() const;
  QPointer<QComboBox> RightComboBox() const;
  QPointer<QGroupBox> RightGroupBox() const;

private:
  void ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
                   CEditorActionBar* pActionBar, qint32 iIndex);

  std::shared_ptr<Ui::CEditorLayoutClassic>                   m_spUi;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::vector<QPointer<QAction>>                              m_vpKeyBindingActions;
  qint32                                                      m_iLastLeftIndex;
  qint32                                                      m_iLastRightIndex;
};

DECLARE_EDITORLAYOUT(CEditorLayoutClassic, CSettings::eClassic)

#endif // CEDITORLAYOUTCLASSIC_H
