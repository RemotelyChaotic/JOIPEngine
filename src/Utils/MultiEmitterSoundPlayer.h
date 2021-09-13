#ifndef CMULTIEMITTERSOUNDPLAYER_H
#define CMULTIEMITTERSOUNDPLAYER_H

#include <QString>
#include <memory>
#include <vector>

namespace QtAV { class AVPlayer; }

class CMultiEmitterSoundPlayer
{
public:
  static const qint32 c_iDefaultNumAutioEmitters;

  CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters = c_iDefaultNumAutioEmitters,
                           const QString& sSoundEffect = QString());
  ~CMultiEmitterSoundPlayer();

  const QString& SoundEffect() const;
  void SetSoundEffect(const QString& sPath);

  bool Muted() const;
  void SetMuted(bool bMuted);

  double Volume() const;
  void SetVolume(double dValue);

  void Play();
  void Stop();

private:
  std::vector<std::unique_ptr<QtAV::AVPlayer>> m_vspPlayers;
  qint32                                       m_iNrSoundEmitters;
  QString                                      m_sSoundEffect;
  qint32                                       m_iLastAutioPlayer;
  bool                                         m_bMuted;
  double                                       m_dVolume;
};

#endif // CMULTIEMITTERSOUNDPLAYER_H
