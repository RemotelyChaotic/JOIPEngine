#ifndef CMULTIEMITTERSOUNDPLAYER_H
#define CMULTIEMITTERSOUNDPLAYER_H

#include <QString>
#include <QStringList>
#include <memory>
#include <vector>

namespace QtAV { class AVPlayer; }

class CMultiEmitterSoundPlayer
{
public:
  static const qint32 c_iDefaultNumAutioEmitters;

  CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters = c_iDefaultNumAutioEmitters,
                           bool bLoadAsynch = true);
  CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters,
                           const QString& sSoundEffect,
                           bool bLoadAsynch = true);
  CMultiEmitterSoundPlayer(qint32 iNrSoundEmitters,
                           const QStringList& sSoundEffects,
                           bool bLoadAsynch = true);
  ~CMultiEmitterSoundPlayer();

  void Resize(qint32 iNewNrSoundEmitters);

  const QStringList& SoundEffects() const;
  void SetSoundEffect(const QString& sPath);
  void SetSoundEffects(const QStringList& sPath);

  bool Muted() const;
  void SetMuted(bool bMuted);

  double Volume() const;
  void SetVolume(double dValue);

  void Play();
  void Stop();

private:
  void AdvanceNextSfxToLoad();
  void Initialize();

  std::vector<std::unique_ptr<QtAV::AVPlayer>> m_vspPlayers;
  qint32                                       m_iNrSoundEmitters;
  QStringList                                  m_sSoundEffects;
  qint32                                       m_iCurrentAutioPlayer;
  qint32                                       m_iNextSfxToLoad;
  bool                                         m_bMuted;
  double                                       m_dVolume;
  bool                                         m_bAsynchLoad;
};

#endif // CMULTIEMITTERSOUNDPLAYER_H
