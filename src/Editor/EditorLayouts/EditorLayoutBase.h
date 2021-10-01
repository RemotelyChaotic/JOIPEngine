#ifndef IEDITORLAYOUT_H
#define IEDITORLAYOUT_H

#include "IEditorLayoutViewProvider.h"
#include "Widgets/IWidgetBaseInterface.h"
#include <QWidget>
#include <memory>
#include <type_traits>

class CEditorModel;
class CEditorWidgetBase;
class ITutorialStateSwitchHandler;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CEditorLayoutBase : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT
  friend class ITutorialStateSwitchHandler;

public:
  explicit CEditorLayoutBase(QWidget* pParent = nullptr);
  ~CEditorLayoutBase() override;

  void Initialize(std::weak_ptr<IEditorLayoutViewProvider> pLayoutViewProvider,
                  QPointer<CEditorModel> pEditorModel);

  virtual void ProjectLoaded(tspProject spCurrentProject, bool bModified) = 0;
  virtual void ProjectUnloaded() = 0;

protected:
  QPointer<CEditorModel> EditorModel() const;
  QPointer<CEditorTutorialOverlay> GetTutorialOverlay() const;
  template<class T, typename std::enable_if<std::is_base_of<CEditorWidgetBase, T>::value, bool>::type = true>
  QPointer<T> GetWidget() const;
  QPointer<CEditorWidgetBase> GetWidget(EEditorWidget widget) const;
  void VisitWidgets(const std::function<void(QPointer<CEditorWidgetBase>, EEditorWidget)>& fnVisitor);

  virtual void InitializeImpl() = 0;

  std::shared_ptr<ITutorialStateSwitchHandler>  m_spStateSwitchHandler;

private:
  void Initialize() override;

  std::weak_ptr<IEditorLayoutViewProvider>      m_pLayoutViewProvider;
  QPointer<CEditorModel>                        m_pEditorModel;
};

#endif // IEDITORLAYOUT_H
