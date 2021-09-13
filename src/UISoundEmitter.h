#ifndef UISOUNDEMITTER_H
#define UISOUNDEMITTER_H

#include <QObject>
#include <memory>

class CMultiEmitterSoundPlayer;
class CSettings;

class CUISoundEmitter : public QObject
{
  Q_OBJECT
public:
  explicit CUISoundEmitter(QObject *parent = nullptr);
  ~CUISoundEmitter() override;

  void Initialize();

protected:
  bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private slots:
  void SlotMutedChanged();
  void SlotVolumeChanged();

private:
  void SetVolume(double dVolume);

  std::unique_ptr<CMultiEmitterSoundPlayer> m_spCklickSoundButton;
  std::unique_ptr<CMultiEmitterSoundPlayer> m_spHoverSoundButton;
  std::unique_ptr<CMultiEmitterSoundPlayer> m_spCklickSoundCheckbox;
  std::unique_ptr<CMultiEmitterSoundPlayer> m_spCklickSoundDropBox;
  std::shared_ptr<CSettings>                m_spSettings;
};

#endif // UISOUNDEMITTER_H
