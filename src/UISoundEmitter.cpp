#include "UISoundEmitter.h"
#include "Application.h"
#include "Settings.h"
#include "Utils/MultiEmitterSoundPlayer.h"
#include <QEvent>
#include <QAbstractButton>
#include <QAbstractItemDelegate>
#include <QCheckBox>
#include <QComboBox>

namespace
{
  const qint32 c_iUiSoundEmitterCount = 3;
}

CUISoundEmitter::CUISoundEmitter(QObject *parent) :
  QObject(parent),
  m_spCklickSoundButton(
    std::make_unique<CMultiEmitterSoundPlayer>(c_iUiSoundEmitterCount,
                                               ":/resources/sound/menu_click_soft_main.wav")),
  m_spHoverSoundButton(
    std::make_unique<CMultiEmitterSoundPlayer>(c_iUiSoundEmitterCount,
                                               ":/resources/sound/menu_selection_soft.wav")),
  m_spCklickSoundCheckbox(
    std::make_unique<CMultiEmitterSoundPlayer>(c_iUiSoundEmitterCount,
                                               ":/resources/sound/menu_selection_soft.wav")),
  m_spCklickSoundDropBox(
    std::make_unique<CMultiEmitterSoundPlayer>(c_iUiSoundEmitterCount,
                                               ":/resources/sound/menu_whoosh.wav"))
{
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
    connect(m_spSettings.get(), &CSettings::mutedChanged,
            this, &CUISoundEmitter::SlotMutedChanged, Qt::QueuedConnection);
    connect(m_spSettings.get(), &CSettings::volumeChanged,
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
        m_spHoverSoundButton->Play();
      }
      else if (nullptr != qobject_cast<QAbstractButton*>(pObject))
      {
        m_spHoverSoundButton->Play();
      }
      else if (nullptr != qobject_cast<QAbstractItemDelegate*>(pObject))
      {
        m_spHoverSoundButton->Play();
      }
    } break;
    case QEvent::HoverLeave:
    {
    } break;
    case QEvent::MouseButtonPress:
    {
      if (nullptr != qobject_cast<QCheckBox*>(pObject))
      {
        m_spCklickSoundCheckbox->Play();
      }
      else if (nullptr != qobject_cast<QComboBox*>(pObject))
      {
        m_spCklickSoundButton->Play();
      }
      else if (nullptr != qobject_cast<QAbstractButton*>(pObject))
      {
        m_spCklickSoundButton->Play();
      }
      else if (nullptr != qobject_cast<QAbstractItemDelegate*>(pObject))
      {
        m_spCklickSoundButton->Play();
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
  m_spCklickSoundButton->SetVolume(dVolume);
  m_spHoverSoundButton->SetVolume(dVolume);
  m_spCklickSoundCheckbox->SetVolume(dVolume);
  m_spCklickSoundDropBox->SetVolume(dVolume);
}
