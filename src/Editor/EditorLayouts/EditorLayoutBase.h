#ifndef IEDITORLAYOUT_H
#define IEDITORLAYOUT_H

#include "IEditorLayoutViewProvider.h"

#include "Editor/EditorWidgetRegistry.h"
#include "Editor/EditorWidgetTypes.h"

#include "Widgets/IWidgetBaseInterface.h"

#include <QPointer>
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
                  QPointer<CEditorModel> pEditorModel,
                  bool bWithTutorial);

  QPointer<CEditorModel> EditorModel() const;

  virtual void ProjectLoaded(tspProject spCurrentProject, bool bModified) = 0;
  virtual void ProjectUnloaded() = 0;

protected:
  QPointer<CEditorTutorialOverlay> GetTutorialOverlay() const;
  template<class T,
           typename std::enable_if<
               std::is_base_of<CEditorWidgetBase, T>::value &&
               !std::is_same_v<CEditorWidgetBase, T>, bool>::type = true>
  QPointer<T> GetWidget() const
  {
    if (auto spLayoutProvider = m_pLayoutViewProvider.lock())
    {
      qint32 iId = detail::SRegistryEntry<T>::m_iId;
      if (EEditorWidget::_is_valid(iId))
      {
        return qobject_cast<T*>(
            spLayoutProvider->GetEditorWidget(EEditorWidget::_from_integral(iId)).data());
      }
    }
    return nullptr;
  }
  QPointer<CEditorWidgetBase> GetWidget(EEditorWidget widget) const;
  void VisitWidgets(const std::function<void(QPointer<CEditorWidgetBase>, EEditorWidget)>& fnVisitor);

  virtual void InitializeImpl(bool bWithTutorial) = 0;

  std::shared_ptr<ITutorialStateSwitchHandler>  m_spStateSwitchHandler;
  std::weak_ptr<IEditorLayoutViewProvider>      m_pLayoutViewProvider;
  QPointer<CEditorModel>                        m_pEditorModel;

private:
  void Initialize() override;

  bool                                          m_bWithTutorial;
};

#endif // IEDITORLAYOUT_H
