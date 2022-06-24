#include "WidgetHelpers.h"

namespace widget_helpers
{
  void RetainSizeAndHide(QWidget* pWidget)
  {
    if (nullptr != pWidget)
    {
      QSizePolicy policy = pWidget->sizePolicy();
      policy.setRetainSizeWhenHidden(true);
      pWidget->setSizePolicy(policy);
      pWidget->hide();
    }
  }
}
