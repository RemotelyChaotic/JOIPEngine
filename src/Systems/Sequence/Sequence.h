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

  std::shared_ptr<SSequenceInstruction> Clone();
  virtual std::shared_ptr<SSequenceInstruction> CloneImpl() = 0;

  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SSingleBeatInstruction : public SSequenceInstruction
{
  std::optional<double>              m_dVolume = std::nullopt;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStartPatternInstruction : public SSequenceInstruction
{
  std::vector<double>                m_vdPattern;
  qint32                             m_iBpm = 60;
  std::optional<QString>             m_sResource = std::nullopt;
  std::optional<double>              m_dVolume = std::nullopt;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopPatternInstruction : public SSequenceInstruction
{
  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
};
struct SVibrateInstruction : public SSequenceInstruction
{
  double                             m_dSpeed = 0.0;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SLinearToyInstruction : public SSequenceInstruction
{
  double                             m_dDurationS = 0.0;
  double                             m_dPosition = 0.0;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SRotateToyInstruction : public SSequenceInstruction
{
  bool                               m_bClockwise = false;
  double                             m_dSpeed = 0.0;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopVibrationsInstruction : public SSequenceInstruction
{
  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
};
struct SShowMediaInstruction : public SSequenceInstruction
{
  QString                            m_sResource;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPlayVideoInstruction : public SSequenceInstruction
{
  QString                            m_sResource;
  std::optional<qint64>              m_iLoops = std::nullopt;
  std::optional<qint64>              m_iStartAt = std::nullopt;
  std::optional<qint64>              m_iEndAt = std::nullopt;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPauseVideoInstruction : public SSequenceInstruction
{
  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
};
struct SStopVideoInstruction : public SSequenceInstruction
{
  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
};
struct SPlayAudioInstruction : public SSequenceInstruction
{
  QString                            m_sResource;
  std::optional<QString>             m_sName = std::nullopt;
  std::optional<qint64>              m_iLoops = std::nullopt;
  std::optional<qint64>              m_iStartAt = std::nullopt;
  std::optional<qint64>              m_iEndAt = std::nullopt;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SPauseAudioInstruction : public SSequenceInstruction
{
  QString                            m_sName;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SStopAudioInstruction : public SSequenceInstruction
{
  QString                            m_sName;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SShowTextInstruction : public SSequenceInstruction
{
  std::optional<QString>             m_sText = std::nullopt;
  std::optional<double>              m_dSkippableWaitS = std::nullopt; // unused
  std::optional<bool>                m_bSkippable = std::nullopt; // unused
  std::optional<QColor>              m_textColor = std::nullopt;
  std::optional<QColor>              m_bgColor = std::nullopt;
  std::optional<QString>             m_sPortrait = std::nullopt;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SRunScriptInstruction : public SSequenceInstruction
{
  QString                            m_sResource;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
  QJsonObject ToJsonObject() override;
  void FromJsonObject(const QJsonObject& json) override;
};
struct SEvalInstruction : public SSequenceInstruction
{
  QString                            m_sScript;

  std::shared_ptr<SSequenceInstruction> CloneImpl() override;
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
