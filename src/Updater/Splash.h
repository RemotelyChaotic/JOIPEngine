#ifndef CSPLASH_H
#define CSPLASH_H

#include "SettingsData.h"

#include <QGroupBox>
#include <QPointer>
#include <QTimer>
#include <QWidget>

class CTitleLabel;
class CUpdater;
class QLabel;

const char c_sSettingAutoUpdate[] = "General/autoUpdate";

class CSplash : public QWidget
{
  Q_OBJECT

public:
  explicit CSplash(const SSettingsData& settings, QWidget *parent = nullptr);
  ~CSplash();

public slots:
  void StartUpdate();
  void StartMainExe();

protected slots:
  void SlotAnimationFinished();
  void SlotReloadText(bool bSkipUpdate = false);

protected:
  void paintEvent(QPaintEvent* pEvt) override;

private:
  SSettingsData         m_settings;
  QPointer<CUpdater>    m_pUpdater;
  QPointer<CTitleLabel> m_pSplashText;
  QPointer<QLabel>      m_pMessageLabel;
  QPointer<QGroupBox>   m_pPopupBox;
  QTimer                m_animationTimer;
  QPixmap               m_backgroundPixmap;
  QColor                m_backgroundColor;
  qint32                m_iNbrDots = 2;
};

#endif // CSPLASH_H
