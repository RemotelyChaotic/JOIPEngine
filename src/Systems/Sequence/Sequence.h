#ifndef SSEQUENCEFILE_H
#define SSEQUENCEFILE_H

#include "Systems/ISerializable.h"

#include <QColor>
#include <QString>
#include <QVariant>

#include <memory>
#include <optional>
#include <vector>

namespace sequence
{
  const char c_sCategoryIdBeat[] = "Metronome Commands";
  const char c_sInstructionIdBeat[] = "Single Beat";
  const char c_sInstructionIdStartPattern[] = "Start Pattern";
  const char c_sInstructionIdStopPattern[] = "Stop Pattern";

  const char c_sCategoryIdToy[] = "Toy Commands";
  const char c_sInstructionIdVibrate[] = "Vibrate";
  const char c_sInstructionIdLinearToy[] = "Linear Toy Command";
  const char c_sInstructionIdRotateToy[] = "Rotate Toy Command";
  const char c_sInstructionIdStopVibrations[] = "Stop Vibrations";

  const char c_sCategoryIdResource[] = "Resource Commands";
  const char c_sInstructionIdShow[] = "Show";
  const char c_sInstructionIdPlayVideo[] = "Play Video";
  const char c_sInstructionIdPauseVideo[] = "Pause Video";
  const char c_sInstructionIdStopVideo[] = "Stop Video";
  const char c_sInstructionIdPlayAudio[] = "Play Audio";
  const char c_sInstructionIdPauseAudio[] = "Pause Audio";
  const char c_sInstructionIdStopAudio[] = "Stop Audio";

  const char c_sCategoryIdText[] = "Text Commands";
  const char c_sInstructionIdShowText[] = "Show Text";

  const char c_sCategoryIdScript[] = "Script Commands";
  const char c_sInstructionIdRunScript[] = "Run Script";
  const char c_sInstructionIdEval[] = "Eval";
}

//----------------------------------------------------------------------------------------
//
struct SSequenceInstruction : public ISerializable
{
  QString                            m_sInstructionType;

  void CopyFrom(const SSequenceInstruction* pOther);
  void CopyFrom(const std::shared_ptr<SSequenceInstruction> spOther);

  virtual std::shared_ptr<SSequenceInstruction> Clone() = 0;
  virtual void CopyFromImpl(const SSequenceInstruction* pOther) = 0;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
template <typename T>
struct SSequenceTypedInstruction : public SSequenceInstruction
{
  std::shared_ptr<SSequenceInstruction> Clone() override final
  {
    std::shared_ptr<SSequenceInstruction> spClone = std::make_shared<T>();
    spClone->m_sInstructionType = m_sInstructionType;
    spClone->CopyFrom(this);
    return spClone;
  }
};
struct SSingleBeatInstruction : public SSequenceTypedInstruction<SSingleBeatInstruction>
{
  std::optional<double>              m_dVolume = std::nullopt;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStartPatternInstruction : public SSequenceTypedInstruction<SStartPatternInstruction>
{
  std::vector<double>                m_vdPattern;
  qint32                             m_iBpm = 60;
  std::optional<QString>             m_sResource = std::nullopt;
  std::optional<double>              m_dVolume = std::nullopt;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopPatternInstruction : public SSequenceTypedInstruction<SStartPatternInstruction>
{
  void CopyFromImpl(const SSequenceInstruction* pOther) override;
};
struct SVibrateInstruction : public SSequenceTypedInstruction<SVibrateInstruction>
{
  double                             m_dSpeed = 0.0;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SLinearToyInstruction : public SSequenceTypedInstruction<SLinearToyInstruction>
{
  double                             m_dDurationS = 0.0;
  double                             m_dPosition = 0.0;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SRotateToyInstruction : public SSequenceTypedInstruction<SRotateToyInstruction>
{
  bool                               m_bClockwise = false;
  double                             m_dSpeed = 0.0;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopVibrationsInstruction : public SSequenceTypedInstruction<SStopVibrationsInstruction>
{
  void CopyFromImpl(const SSequenceInstruction* pOther) override;
};
struct SShowMediaInstruction : public SSequenceTypedInstruction<SShowMediaInstruction>
{
  QString                            m_sResource;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPlayVideoInstruction : public SSequenceTypedInstruction<SPlayVideoInstruction>
{
  QString                            m_sResource;
  std::optional<qint64>              m_iLoops = std::nullopt;
  std::optional<qint64>              m_iStartAt = std::nullopt;
  std::optional<qint64>              m_iEndAt = std::nullopt;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPauseVideoInstruction : public SSequenceTypedInstruction<SPauseVideoInstruction>
{
  void CopyFromImpl(const SSequenceInstruction* pOther) override;
};
struct SStopVideoInstruction : public SSequenceTypedInstruction<SStopVideoInstruction>
{
  void CopyFromImpl(const SSequenceInstruction* pOther) override;
};
struct SPlayAudioInstruction : public SSequenceTypedInstruction<SPlayAudioInstruction>
{
  QString                            m_sResource;
  std::optional<QString>             m_sName = std::nullopt;
  std::optional<qint64>              m_iLoops = std::nullopt;
  std::optional<qint64>              m_iStartAt = std::nullopt;
  std::optional<qint64>              m_iEndAt = std::nullopt;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPauseAudioInstruction : public SSequenceTypedInstruction<SPauseAudioInstruction>
{
  QString                            m_sName;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopAudioInstruction : public SSequenceTypedInstruction<SStopAudioInstruction>
{
  QString                            m_sName;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SShowTextInstruction : public SSequenceTypedInstruction<SShowTextInstruction>
{
  std::optional<QString>             m_sText = std::nullopt;
  std::optional<double>              m_dSkippableWaitS = std::nullopt; // unused
  std::optional<bool>                m_bSkippable = std::nullopt; // unused
  std::optional<QColor>              m_textColor = std::nullopt;
  std::optional<QColor>              m_bgColor = std::nullopt;
  std::optional<QString>             m_sPortrait = std::nullopt;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SRunScriptInstruction : public SSequenceTypedInstruction<SRunScriptInstruction>
{
  QString                            m_sResource;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SEvalInstruction : public SSequenceTypedInstruction<SEvalInstruction>
{
  QString                            m_sScript;

  void CopyFromImpl(const SSequenceInstruction* pOther) override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
namespace sequence
{
  using tTimePos = qint64;
  using tTimedInstruction = std::pair<sequence::tTimePos,
                                      std::shared_ptr<SSequenceInstruction>>;

  std::shared_ptr<SSequenceInstruction> CreateInstruction(const QString& sId,
                                                          const QVariantList& vArgs);

  qint64 TimeOffsetFromInstructionType(const QString& sId);
}

//----------------------------------------------------------------------------------------
//
struct SSequenceLayer : public ISerializable
{
  QString                                  m_sLayerType;
  QString                                  m_sName;
  std::vector<sequence::tTimedInstruction> m_vspInstructions;

  SSequenceLayer();
  SSequenceLayer(const SSequenceLayer& other);

  std::shared_ptr<SSequenceLayer> Clone();
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
struct SSequenceFile : public ISerializable
{
  std::vector<std::shared_ptr<SSequenceLayer>> m_vspLayers;
  qint64                                       m_iLengthMili = 15*1000;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};

//----------------------------------------------------------------------------------------
//
typedef std::shared_ptr<SSequenceLayer> tspSequenceLayer;
typedef std::shared_ptr<SSequenceFile> tspSequence;

#endif // SSEQUENCEFILE_H
