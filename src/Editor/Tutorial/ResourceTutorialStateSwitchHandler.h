#ifndef CRESOURCETUTORIALSTATESWITCHHANDLER_H
#define CRESOURCETUTORIALSTATESWITCHHANDLER_H

#include "ITutorialStateSwitchHandler.h"
#include <QPointer>
#include <memory>

class CEditorResourceWidget;
namespace Ui {
  class CEditorResourceWidget;
}

class CResourceTutorialStateSwitchHandler : public QObject, public ITutorialStateSwitchHandler
{
  Q_OBJECT

public:
  CResourceTutorialStateSwitchHandler(QPointer<CEditorResourceWidget> pParentWidget,
                                      const std::shared_ptr<Ui::CEditorResourceWidget>& spUi);
  ~CResourceTutorialStateSwitchHandler() override;

  void OnResetStates() override;
  void OnStateSwitch(ETutorialState newState, ETutorialState oldstate) override;

protected slots:
  void SlotCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

private:
  QPointer<CEditorResourceWidget>                  m_pParentWidget;
  std::shared_ptr<Ui::CEditorResourceWidget>       m_spUi;
  ETutorialState                                   m_currentState;
  QMetaObject::Connection                          m_connection;
};

#endif // CRESOURCETUTORIALSTATESWITCHHANDLER_H
