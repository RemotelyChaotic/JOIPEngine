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

  void ChangeIndex(QComboBox* pComboBox, QWidget* pContainer,
                   CEditorActionBar* pActionBar, qint32 iIndex);

  QPointer<CEditorActionBar> LeftActionBar() const;
  QPointer<QComboBox>        LeftComboBox() const;
  QPointer<QWidget>          LeftContainer() const;
  QPointer<QGroupBox>        LeftGroupBox() const;
  QPointer<CEditorActionBar> RightActionBar() const;
  QPointer<QComboBox>        RightComboBox() const;
  QPointer<QWidget>          RightContainer() const;
  QPointer<QGroupBox>        RightGroupBox() const;

protected slots:
  void on_pLeftComboBox_currentIndexChanged(qint32 iIndex);
  void on_pRightComboBox_currentIndexChanged(qint32 iIndex);
  void SlotDisplayResource(const QString& sName);
  void SlotKeyBindingsChanged();

protected:
  void InitializeImpl(bool bWithTutorial) override;

private:
  std::shared_ptr<Ui::CEditorLayoutClassic>                   m_spUi;
  tspProject                                                  m_spCurrentProject;
  std::weak_ptr<CDatabaseManager>                             m_wpDbManager;
  std::vector<QPointer<QAction>>                              m_vpKeyBindingActions;
  qint32                                                      m_iLastLeftIndex;
  qint32                                                      m_iLastRightIndex;
};

DECLARE_EDITORLAYOUT(CEditorLayoutClassic, CSettings::eClassic)

#endif // CEDITORLAYOUTCLASSIC_H
