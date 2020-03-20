#ifndef HELPOVERLAY_H
#define HELPOVERLAY_H

#include "OverlayBase.h"
#include <QWidget>
#include <memory>

class CHelpFactory;
namespace Ui {
  class CHelpOverlay;
}

class CHelpOverlay : public COverlayBase
{
  Q_OBJECT

public:
  explicit CHelpOverlay(QWidget* pParent = nullptr);
  ~CHelpOverlay() override;

private:
  std::unique_ptr<Ui::CHelpOverlay> m_spUi;
  std::weak_ptr<CHelpFactory>       m_wpHelpFactory;
};

#endif // HELPOVERLAY_H
