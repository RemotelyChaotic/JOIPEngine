#ifndef IEDITORLAYOUTVIEWPROVIDER_H
#define IEDITORLAYOUTVIEWPROVIDER_H

#include "Editor/EditorWidgetTypes.h"
#include <QPointer>
#include <functional>

class CEditorTutorialOverlay;
class CEditorWidgetBase;

class IEditorLayoutViewProvider
{
public:
  virtual ~IEditorLayoutViewProvider() {}

  virtual QPointer<CEditorTutorialOverlay> GetTutorialOverlay() const = 0;
  virtual QPointer<CEditorWidgetBase> GetEditorWidget(EEditorWidget widget) const = 0;
  virtual void VisitWidgets(const std::function<void(QPointer<CEditorWidgetBase>, EEditorWidget)>& fnVisitor) = 0;

protected:
  IEditorLayoutViewProvider() {}
};

#endif // IEDITORLAYOUTVIEWPROVIDER_H
