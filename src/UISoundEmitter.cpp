#include "UISoundEmitter.h"
#include "Application.h"
#include "Settings.h"
#include <QEvent>
#include <QAbstractButton>
#include <QAbstractItemDelegate>
#include <QCheckBox>
#include <QComboBox>
#include <QSoundEffect>

CUISoundEmitter::CUISoundEmitter(QObject *parent) :
  QObject(parent),
  m_spCklickSoundButton(std::make_unique<QSoundEffect>()),
  m_spHoverSoundButton(std::make_unique<QSoundEffect>()),
  m_spCklickSoundCheckbox(std::make_unique<QSoundEffect>()),
  m_spCklickSoundDropBox(std::make_unique<QSoundEffect>())
{
  m_spCklickSoundButton->setSource(QUrl("qrc:/resources/sound/menu_click_soft_main.wav"));
  m_spHoverSoundButton->setSource(QUrl("qrc:/resources/sound/menu_selection_soft.wav"));
  m_spCklickSoundCheckbox->setSource(QUrl("qrc:/resources/sound/menu_click_sharp.wav"));
  m_spCklickSoundDropBox->setSource(QUrl("qrc:/resources/sound/menu_whoosh.wav"));

  m_spCklickSoundButton->setLoopCount(1);
  m_spHoverSoundButton->setLoopCount(1);
  m_spCklickSoundCheckbox->setLoopCount(1);
  m_spCklickSoundDropBox->setLoopCount(1);
}

CUISoundEmitter::~CUISoundEmitter()
{

}

//----------------------------------------------------------------------------------------
//
void CUISoundEmitter::Initialize()
{
  m_spSettings = CApplication::Instance()->Settings();
  if (nullptr != m_spSettings)
  {
    connect(m_spSettings.get(), &CSettings::MutedChanged,
            this, &CUISoundEmitter::SlotMutedChanged, Qt::QueuedConnection);
    connect(m_spSettings.get(), &CSettings::VolumeChanged,
            this, &CUISoundEmitter::SlotVolumeChanged, Qt::QueuedConnection);

    SlotVolumeChanged();
  }
}

//----------------------------------------------------------------------------------------
//
bool CUISoundEmitter::eventFilter(QObject* pObject, QEvent* pEvent)
{
  if (nullptr == pEvent || nullptr == pObject)
  {
    return false;
  }

  switch(pEvent->type())
  {
    case QEvent::HoverEnter:
    {
      if (nullptr != qobject_cast<QComboBox*>(pObject))
      {
        m_spHoverSoundButton->stop();
        m_spHoverSoundButton->play();
      }
      else if (nullptr != qobject_cast<QAbstractButton*>(pObject))
      {
        m_spHoverSoundButton->stop();
        m_spHoverSoundButton->play();
      }
      else if (nullptr != qobject_cast<QAbstractItemDelegate*>(pObject))
      {
        m_spHoverSoundButton->stop();
        m_spHoverSoundButton->play();
      }
    } break;
    case QEvent::HoverLeave:
    {
    } break;
    case QEvent::MouseButtonPress:
    {
      if (nullptr != qobject_cast<QCheckBox*>(pObject))
      {
        m_spCklickSoundCheckbox->stop();
        m_spCklickSoundCheckbox->play();
      }
      else if (nullptr != qobject_cast<QComboBox*>(pObject))
      {
        m_spCklickSoundButton->stop();
        m_spCklickSoundButton->play();
      }
      else if (nullptr != qobject_cast<QAbstractButton*>(pObject))
      {
        m_spCklickSoundButton->stop();
        m_spCklickSoundButton->play();
      }
      else if (nullptr != qobject_cast<QAbstractItemDelegate*>(pObject))
      {
        m_spCklickSoundButton->stop();
        m_spCklickSoundButton->play();
      }
    } break;
  default: break;
  }

  return false;
}

//----------------------------------------------------------------------------------------
//
void CUISoundEmitter::SlotMutedChanged()
{
  if (nullptr != m_spSettings)
  {
    SetVolume(m_spSettings->Muted() ? 0.0 : m_spSettings->Volume());
  }
}

//----------------------------------------------------------------------------------------
//
void CUISoundEmitter::SlotVolumeChanged()
{
  if (nullptr != m_spSettings)
  {
    SetVolume(m_spSettings->Muted() ? 0.0 : m_spSettings->Volume());
  }
}

//----------------------------------------------------------------------------------------
//
void CUISoundEmitter::SetVolume(double dVolume)
{
  m_spCklickSoundButton->setVolume(dVolume);
  m_spHoverSoundButton->setVolume(dVolume);
  m_spCklickSoundCheckbox->setVolume(dVolume);
  m_spCklickSoundDropBox->setVolume(dVolume);
}
