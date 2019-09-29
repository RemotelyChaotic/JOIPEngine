#ifndef INFORMATIONWIDGET_H
#define INFORMATIONWIDGET_H

#include "Widgets/IWidgetBaseInterface.h"
#include <QBitmap>
#include <QLabel>
#include <QPointer>
#include <QTimer>
#include <QWidget>
#include <map>
#include <memory>

class CSettings;
namespace Ui {
  class CInformationWidget;
}
struct SResource;
typedef std::shared_ptr<SResource> tspResource;

class CInformationWidget : public QWidget, public IWidgetBaseInterface
{
  Q_OBJECT

public:
  explicit CInformationWidget(QWidget* pParent = nullptr);
  ~CInformationWidget() override;

  void Initialize() override;

signals:
  void SignalError(QString sError, QtMsgType type);
  void SignalQuit();
  void SignalWaitSkipped();

public slots:
  void SlotHideIcon(QString sIconIdentifier);
  void SlotShowIcon(tspResource spResource);
  void SlotShowSkipIcon(qint32 iTimeS);
  void SlotSkipTimeout();

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
  void AddIcon(const QString& sName, const tspResource& spResource);
  QWidget* CreateHeaderIcon(QWidget* pParent, const QString& sName, const QString sPath);
  void RemoveAllIcons();
  void RemoveIcon(const QString& sName);

  std::unique_ptr<Ui::CInformationWidget> m_spUi;
  std::shared_ptr<CSettings>              m_spSettings;
  QPointer<QWidget>                       m_pExitWidget;
  QPointer<QWidget>                       m_pSkipWidget;
  std::map<QString, QPointer<QWidget>>    m_iconMap;
  QTimer                                  m_skipTimer;
  QBitmap                                 m_iconMask;
};

#endif // INFORMATIONWIDGET_H
