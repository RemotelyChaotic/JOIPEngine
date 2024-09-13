#include "Sequence.h"

#include <QJsonArray>

#include <functional>
#include <map>

namespace
{
  std::shared_ptr<SSequenceInstruction> CreateBeatInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SSingleBeatInstruction> spInstr = std::make_shared<SSingleBeatInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdBeat;
    if (vArgs.size() > 0)
    {
      bool bOk = false;
      double dVal = vArgs[0].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dVolume = dVal;
      }
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateStartPatternInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SStartPatternInstruction> spInstr = std::make_shared<SStartPatternInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdStartPattern;
    if (vArgs.size() > 0)
    {
      QVariantList varList = vArgs[0].toList();
      for (const QVariant& v : qAsConst(varList))
      {
        bool bOk = false;
        double dVal = v.toDouble(&bOk);
        if (bOk)
        {
          spInstr->m_vdPattern.push_back(dVal);
        }
      }
    }
    if (vArgs.size() > 1 && vArgs[1].type() == QVariant::Int)
    {
      spInstr->m_iBpm = vArgs[1].toInt();
    }
    qint32 iCounter = 2;
    if (vArgs.size() > iCounter && vArgs[iCounter].type() == QVariant::StringList)
    {
      spInstr->m_vsResources = vArgs[iCounter].toStringList();
      iCounter++;
    }
    if (vArgs.size() > iCounter && vArgs[iCounter].type() == QVariant::Double)
    {
      bool bOk = false;
      double dVal = vArgs[iCounter].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dVolume = dVal;
      }
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateStopPatternInstruction(const QVariantList&)
  {
    std::shared_ptr<SStopPatternInstruction> spInstr = std::make_shared<SStopPatternInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdStopPattern;
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateVibrateInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SVibrateInstruction> spInstr = std::make_shared<SVibrateInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdVibrate;
    if (vArgs.size() > 0)
    {
      bool bOk = false;
      double dVal = vArgs[0].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dSpeed = dVal;
      }
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateLinearToyInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SLinearToyInstruction> spInstr = std::make_shared<SLinearToyInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdLinearToy;
    if (vArgs.size() > 0)
    {
      bool bOk = false;
      double dVal = vArgs[0].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dDurationS = dVal;
      }
    }
    if (vArgs.size() > 1)
    {
      bool bOk = false;
      double dVal = vArgs[1].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dPosition = dVal;
      }
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateRotateToyInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SRotateToyInstruction> spInstr = std::make_shared<SRotateToyInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdRotateToy;
    if (vArgs.size() > 0)
    {
      bool bOk = false;
      bool bVal = vArgs[0].toBool();
      if (bOk)
      {
        spInstr->m_bClockwise = bVal;
      }
    }
    if (vArgs.size() > 1)
    {
      bool bOk = false;
      double dVal = vArgs[1].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dSpeed = dVal;
      }
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateStopVibrateInstruction(const QVariantList&)
  {
    std::shared_ptr<SStopVibrationsInstruction> spInstr = std::make_shared<SStopVibrationsInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdStopVibrations;
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateShowInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SShowMediaInstruction> spInstr = std::make_shared<SShowMediaInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdShow;
    if (vArgs.size() > 0)
    {
      spInstr->m_sResource = vArgs[0].toString();
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreatePlayVideoInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SPlayVideoInstruction> spInstr = std::make_shared<SPlayVideoInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdPlayVideo;
    if (vArgs.size() > 0)
    {
      spInstr->m_sResource = vArgs[0].toString();
    }
    qint32 iCurrent = 1;
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iLoops = iVal;
      }
      iCurrent++;
    }
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iStartAt = iVal;
      }
      iCurrent++;
    }
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iEndAt = iVal;
      }
      iCurrent++;
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreatePauseVideoInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SPauseVideoInstruction> spInstr = std::make_shared<SPauseVideoInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdPauseVideo;
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateStopVideoInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SStopVideoInstruction> spInstr = std::make_shared<SStopVideoInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdStopVideo;
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreatePlayAudioInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SPlayAudioInstruction> spInstr = std::make_shared<SPlayAudioInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdPlayAudio;
    if (vArgs.size() > 0)
    {
      spInstr->m_sResource = vArgs[0].toString();
    }
    qint32 iCurrent = 1;
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::String)
    {
      spInstr->m_sName = vArgs[iCurrent].toString();
      iCurrent++;
    }
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iLoops = iVal;
      }
      iCurrent++;
    }
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iStartAt = iVal;
      }
      iCurrent++;
    }
    if (vArgs.size() > iCurrent && vArgs[iCurrent].type() == QVariant::LongLong)
    {
      bool bOk = false;
      qint64 iVal = vArgs[iCurrent].toLongLong(&bOk);
      if (bOk)
      {
        spInstr->m_iEndAt = iVal;
      }
      iCurrent++;
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreatePauseAudioInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SPauseAudioInstruction> spInstr = std::make_shared<SPauseAudioInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdPauseAudio;
    if (vArgs.size() > 0)
    {
      spInstr->m_sName = vArgs[0].toString();
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateStopAudioInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SStopAudioInstruction> spInstr = std::make_shared<SStopAudioInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdStopAudio;
    if (vArgs.size() > 0)
    {
      spInstr->m_sName = vArgs[0].toString();
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateTextInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SShowTextInstruction> spInstr = std::make_shared<SShowTextInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdShowText;
    if (vArgs.size() > 0)
    {
      spInstr->m_sText = vArgs[0].toString();
    }
    if (vArgs.size() > 1)
    {
      bool bOk = false;
      double dVal = vArgs[1].toDouble(&bOk);
      if (bOk)
      {
        spInstr->m_dSkippableWaitS = dVal;
      }
    }
    if (vArgs.size() > 2)
    {
      spInstr->m_bSkippable = vArgs[2].toBool();
    }
    if (vArgs.size() > 3)
    {
      spInstr->m_textColor = vArgs[3].value<QColor>();
    }
    if (vArgs.size() > 4)
    {
      spInstr->m_bgColor = vArgs[4].value<QColor>();
    }
    if (vArgs.size() > 5)
    {
      spInstr->m_sPortrait = vArgs[5].toString();
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateRunScriptInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SRunScriptInstruction> spInstr = std::make_shared<SRunScriptInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdRunScript;
    if (vArgs.size() > 0)
    {
      spInstr->m_sResource = vArgs[0].toString();
    }
    return spInstr;
  }

  std::shared_ptr<SSequenceInstruction> CreateEvalInstruction(const QVariantList& vArgs)
  {
    std::shared_ptr<SEvalInstruction> spInstr = std::make_shared<SEvalInstruction>();
    spInstr->m_sInstructionType = sequence::c_sInstructionIdEval;
    if (vArgs.size() > 0)
    {
      spInstr->m_sScript = vArgs[0].toString();
    }
    return spInstr;
  }

  //--------------------------------------------------------------------------------------
  //
  using tSeqFactoryFn = std::function<std::shared_ptr<SSequenceInstruction>(const QVariantList&)>;
  const std::map<QString, tSeqFactoryFn>& GetFactoryFunctionMap()
  {
    static std::map<QString, tSeqFactoryFn> fnMap =
    {
      { QString(sequence::c_sInstructionIdBeat), CreateBeatInstruction },
      { QString(sequence::c_sInstructionIdStartPattern), CreateStartPatternInstruction },
      { QString(sequence::c_sInstructionIdStopPattern), CreateStopPatternInstruction },

      { QString(sequence::c_sInstructionIdVibrate), CreateVibrateInstruction },
      { QString(sequence::c_sInstructionIdLinearToy), CreateLinearToyInstruction },
      { QString(sequence::c_sInstructionIdRotateToy), CreateRotateToyInstruction },
      { QString(sequence::c_sInstructionIdStopVibrations), CreateStopVibrateInstruction },

      { QString(sequence::c_sInstructionIdShow), CreateShowInstruction },
      { QString(sequence::c_sInstructionIdPlayVideo), CreatePlayVideoInstruction },
      { QString(sequence::c_sInstructionIdPauseVideo), CreatePauseVideoInstruction },
      { QString(sequence::c_sInstructionIdStopVideo), CreateStopVideoInstruction },
      { QString(sequence::c_sInstructionIdPlayAudio), CreatePlayAudioInstruction },
      { QString(sequence::c_sInstructionIdPauseAudio), CreatePauseAudioInstruction },
      { QString(sequence::c_sInstructionIdStopAudio), CreateStopAudioInstruction },

      { QString(sequence::c_sInstructionIdShowText), CreateTextInstruction },

      { QString(sequence::c_sInstructionIdRunScript), CreateRunScriptInstruction },
      { QString(sequence::c_sInstructionIdEval), CreateEvalInstruction },
    };
    return fnMap;
  }
}

//--------------------------------------------------------------------------------------
//
void SSequenceInstruction::CopyFrom(const SSequenceInstruction* pOther)
{
  m_sInstructionType = pOther->m_sInstructionType;
  CopyFromImpl(pOther);
}
void SSequenceInstruction::CopyFrom(const std::shared_ptr<SSequenceInstruction> spOther)
{
  CopyFrom(spOther.get());
}
QJsonObject SSequenceInstruction::ToJsonObject()
{
  return {
    { "sInstructionType", m_sInstructionType }
  };
}
void SSequenceInstruction::FromJsonObject(const QJsonObject& json)
{
  auto it = json.find("sInstructionType");
  if (it != json.end())
  {
    m_sInstructionType = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SSingleBeatInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SSingleBeatInstruction* pOtherCasted =
      dynamic_cast<const SSingleBeatInstruction*>(pOther);
  if (nullptr == pOtherCasted) { return; }
  m_dVolume = pOtherCasted->m_dVolume;
}
QJsonObject SSingleBeatInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  if (m_dVolume.has_value())
  {
    obj["dVolume"] = *m_dVolume;
  }
  return obj;
}
void SSingleBeatInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("dVolume");
  if (it != json.end())
  {
    m_dVolume = it.value().toDouble();
  }
}

//--------------------------------------------------------------------------------------
//
void SStartPatternInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SStartPatternInstruction* pOtherCasted =
      dynamic_cast<const SStartPatternInstruction*>(pOther);
  if (nullptr == pOtherCasted) { return; }
  m_vdPattern = pOtherCasted->m_vdPattern;
  m_iBpm = pOtherCasted->m_iBpm;
  m_vsResources = pOtherCasted->m_vsResources;
  m_dVolume = pOtherCasted->m_dVolume;
}
QJsonObject SStartPatternInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  QJsonArray pattern;
  QJsonArray resources;
  for (double dVal : m_vdPattern)
  {
    pattern.push_back(dVal);
  }
  if (m_vsResources.has_value())
  {
    for (const QString& sValue : *m_vsResources)
    {
      resources.push_back(sValue);
    }
  }
  obj["vdPattern"] = pattern;
  obj["iBpm"] = m_iBpm;
  if (m_vsResources.has_value())
  {
    obj["sResources"] = resources;
  }
  if (m_dVolume.has_value())
  {
    obj["dVolume"] = *m_dVolume;
  }
  return obj;
}
void SStartPatternInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  m_vdPattern.clear();
  m_vsResources = std::nullopt;
  auto it = json.find("vdPattern");
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      double dVal = val.toDouble();
      m_vdPattern.push_back(dVal);
    }
  }
  it = json.find("iBpm");
  if (it != json.end())
  {
    m_iBpm = it.value().toInt();
  }
  it = json.find("sResource");
  if (it != json.end())
  {
    m_vsResources = QStringList() << it.value().toString();
  }
  it = json.find("sResources");
  if (it != json.end())
  {
    QStringList vsResources;
    for (QJsonValue val : it.value().toArray())
    {
      QString sVal = val.toString();
      vsResources.push_back(sVal);
    }
    m_vsResources = vsResources;
  }
  it = json.find("dVolume");
  if (it != json.end())
  {
    m_dVolume = it.value().toDouble();
  }
}

//--------------------------------------------------------------------------------------
//
void SStopPatternInstruction::CopyFromImpl(const SSequenceInstruction*)
{
}

//--------------------------------------------------------------------------------------
//
void SVibrateInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SVibrateInstruction* pOtherCasted =
      dynamic_cast<const SVibrateInstruction*>(pOther);
  if (nullptr == pOtherCasted) { return; }
  m_dSpeed = pOtherCasted->m_dSpeed;
}
QJsonObject SVibrateInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["dSpeed"] = m_dSpeed;
  return obj;
}
void SVibrateInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("dSpeed");
  if (it != json.end())
  {
    m_dSpeed = it.value().toDouble();
  }
}

//--------------------------------------------------------------------------------------
//
void SLinearToyInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SLinearToyInstruction* pOtherCasted =
      dynamic_cast<const SLinearToyInstruction*>(pOther);
  if (nullptr == pOtherCasted) { return; }
  m_dDurationS = pOtherCasted->m_dDurationS;
  m_dPosition = pOtherCasted->m_dPosition;
}
QJsonObject SLinearToyInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["dDurationS"] = m_dDurationS;
  obj["dPosition"] = m_dPosition;
  return obj;
}
void SLinearToyInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("dDurationS");
  if (it != json.end())
  {
    m_dDurationS = it.value().toDouble();
  }
  it = json.find("dPosition");
  if (it != json.end())
  {
    m_dPosition = it.value().toDouble();
  }
}

//--------------------------------------------------------------------------------------
//
void SRotateToyInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SRotateToyInstruction* pOtherCasted =
      dynamic_cast<const SRotateToyInstruction*>(pOther);
  m_bClockwise = pOtherCasted->m_bClockwise;
  m_dSpeed = pOtherCasted->m_dSpeed;
}
QJsonObject SRotateToyInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["bClockwise"] = m_bClockwise;
  obj["dSpeed"] = m_dSpeed;
  return obj;
}
void SRotateToyInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("bClockwise");
  if (it != json.end())
  {
    m_bClockwise = it.value().toDouble();
  }
  it = json.find("dSpeed");
  if (it != json.end())
  {
    m_dSpeed = it.value().toDouble();
  }
}

//--------------------------------------------------------------------------------------
//
void SStopVibrationsInstruction::CopyFromImpl(const SSequenceInstruction*)
{
}

//--------------------------------------------------------------------------------------
//
void SShowMediaInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SShowMediaInstruction* pOtherCasted =
      dynamic_cast<const SShowMediaInstruction*>(pOther);
  m_sResource = pOtherCasted-> m_sResource;
}
QJsonObject SShowMediaInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sResource"] = m_sResource;
  return obj;
}
void SShowMediaInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sResource");
  if (it != json.end())
  {
    m_sResource = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SPlayVideoInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SPlayVideoInstruction* pOtherCasted =
      dynamic_cast<const SPlayVideoInstruction*>(pOther);
  m_sResource = pOtherCasted->m_sResource;
  m_iLoops = pOtherCasted->m_iLoops;
  m_iStartAt = pOtherCasted->m_iStartAt;
  m_iEndAt = pOtherCasted->m_iEndAt;
}
QJsonObject SPlayVideoInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sResource"] = m_sResource;
  if (m_iLoops.has_value())
  {
    obj["iLoops"] = double(*m_iLoops);
  }
  if (m_iStartAt.has_value())
  {
    obj["iStartAt"] = double(*m_iStartAt);
  }
  if (m_iEndAt.has_value())
  {
    obj["iEndAt"] = double(*m_iEndAt);
  }
  return obj;
}
void SPlayVideoInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sResource");
  if (it != json.end())
  {
    m_sResource = it.value().toString();
  }
  it = json.find("iLoops");
  if (it != json.end())
  {
    m_iLoops = static_cast<qint64>(it.value().toDouble());
  }
  it = json.find("iStartAt");
  if (it != json.end())
  {
    m_iStartAt = static_cast<qint64>(it.value().toDouble());
  }
  it = json.find("iEndAt");
  if (it != json.end())
  {
    m_iEndAt = static_cast<qint64>(it.value().toDouble());
  }
}

//--------------------------------------------------------------------------------------
//
void SPauseVideoInstruction::CopyFromImpl(const SSequenceInstruction*)
{
}

//--------------------------------------------------------------------------------------
//
void SStopVideoInstruction::CopyFromImpl(const SSequenceInstruction*)
{
}

//--------------------------------------------------------------------------------------
//
void SPlayAudioInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SPlayAudioInstruction* pOtherCasted =
      dynamic_cast<const SPlayAudioInstruction*>(pOther);
  m_sResource = pOtherCasted->m_sResource;
  m_sName = pOtherCasted->m_sName;
  m_iLoops = pOtherCasted->m_iLoops;
  m_iStartAt = pOtherCasted->m_iStartAt;
  m_iEndAt = pOtherCasted->m_iEndAt;
}
QJsonObject SPlayAudioInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sResource"] = m_sResource;
  if (m_sName.has_value())
  {
    obj["sName"] = *m_sName;
  }
  if (m_iLoops.has_value())
  {
    obj["iLoops"] = double(*m_iLoops);
  }
  if (m_iStartAt.has_value())
  {
    obj["iStartAt"] = double(*m_iStartAt);
  }
  if (m_iEndAt.has_value())
  {
    obj["iEndAt"] = double(*m_iEndAt);
  }
  return obj;
}
void SPlayAudioInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sResource");
  if (it != json.end())
  {
    m_sResource = it.value().toString();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("iLoops");
  if (it != json.end())
  {
    m_iLoops = static_cast<qint64>(it.value().toDouble());
  }
  it = json.find("iStartAt");
  if (it != json.end())
  {
    m_iStartAt = static_cast<qint64>(it.value().toDouble());
  }
  it = json.find("iEndAt");
  if (it != json.end())
  {
    m_iEndAt = static_cast<qint64>(it.value().toDouble());
  }
}

//--------------------------------------------------------------------------------------
//
void SPauseAudioInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SPauseAudioInstruction* pOtherCasted =
      dynamic_cast<const SPauseAudioInstruction*>(pOther);
  m_sName = pOtherCasted->m_sName;
}
QJsonObject SPauseAudioInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sName"] = m_sName;
  return obj;
}
void SPauseAudioInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SStopAudioInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SStopAudioInstruction* pOtherCasted =
      dynamic_cast<const SStopAudioInstruction*>(pOther);
  m_sName = pOtherCasted->m_sName;
}
QJsonObject SStopAudioInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sName"] = m_sName;
  return obj;
}
void SStopAudioInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SShowTextInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SShowTextInstruction* pOtherCasted =
      dynamic_cast<const SShowTextInstruction*>(pOther);
  m_sText = pOtherCasted->m_sText;
  m_dSkippableWaitS = pOtherCasted->m_dSkippableWaitS;
  m_bSkippable = pOtherCasted->m_bSkippable;
  m_textColor = pOtherCasted->m_textColor;
  m_bgColor = pOtherCasted->m_bgColor;
  m_sPortrait = pOtherCasted->m_sPortrait;
}
QJsonObject SShowTextInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  if (m_sText.has_value())
  {
    obj["sText"] = m_sText.value();
  }
  if (m_dSkippableWaitS.has_value())
  {
    obj["dSkippableWaitS"] = *m_dSkippableWaitS;
  }
  if (m_bSkippable.has_value())
  {
    obj["bSkippable"] = *m_bSkippable;
  }
  if (m_textColor.has_value())
  {
    obj["textColor_r"] = m_textColor->red();
    obj["textColor_g"] = m_textColor->green();
    obj["textColor_b"] = m_textColor->blue();
    obj["textColor_a"] = m_textColor->alpha();
  }
  if (m_bgColor.has_value())
  {
    obj["bgColor_r"] = m_bgColor->red();
    obj["bgColor_g"] = m_bgColor->green();
    obj["bgColor_b"] = m_bgColor->blue();
    obj["bgColor_a"] = m_bgColor->alpha();
  }
  if (m_sPortrait.has_value())
  {
    obj["sPortrait"] = *m_sPortrait;
  }
  return obj;
}
void SShowTextInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sText");
  if (it != json.end())
  {
    m_sText = it.value().toString();
  }
  it = json.find("dSkippableWaitS");
  if (it != json.end())
  {
    m_dSkippableWaitS = it.value().toDouble();
  }
  it = json.find("bSkippable");
  if (it != json.end())
  {
    m_bSkippable = it.value().toBool();
  }
  auto it1 = json.find("textColor_r");
  auto it2 = json.find("textColor_g");
  auto it3 = json.find("textColor_b");
  auto it4 = json.find("textColor_a");
  if (it1 != json.end() && it2 != json.end() && it3 != json.end() && it4 != json.end())
  {
    m_textColor = QColor(it1.value().toInt(), it2.value().toInt(),
                         it3.value().toInt(), it4.value().toInt());
  }
  it1 = json.find("bgColor_r");
  it2 = json.find("bgColor_g");
  it3 = json.find("bgColor_b");
  it4 = json.find("bgColor_a");
  if (it1 != json.end() && it2 != json.end() && it3 != json.end() && it4 != json.end())
  {
    m_bgColor = QColor(it1.value().toInt(), it2.value().toInt(),
                       it3.value().toInt(), it4.value().toInt());
  }
  it = json.find("sPortrait");
  if (it != json.end())
  {
    m_sPortrait = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SRunScriptInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SRunScriptInstruction* pOtherCasted =
      dynamic_cast<const SRunScriptInstruction*>(pOther);
  m_sResource = pOtherCasted->m_sResource;
}
QJsonObject SRunScriptInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sResource"] = m_sResource;
  return obj;
}
void SRunScriptInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sResource");
  if (it != json.end())
  {
    m_sResource = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
void SEvalInstruction::CopyFromImpl(const SSequenceInstruction* pOther)
{
  const SEvalInstruction* pOtherCasted =
      dynamic_cast<const SEvalInstruction*>(pOther);
  m_sScript = pOtherCasted->m_sScript;
}
QJsonObject SEvalInstruction::ToJsonObject()
{
  QJsonObject obj = SSequenceInstruction::ToJsonObject();
  obj["sScript"] = m_sScript;
  return obj;
}
void SEvalInstruction::FromJsonObject(const QJsonObject& json)
{
  SSequenceInstruction::FromJsonObject(json);
  auto it = json.find("sScript");
  if (it != json.end())
  {
    m_sScript = it.value().toString();
  }
}

//--------------------------------------------------------------------------------------
//
SSequenceLayer::SSequenceLayer()
{
}
SSequenceLayer::SSequenceLayer(const SSequenceLayer& other) :
  m_sLayerType(other.m_sLayerType),
  m_sName(other.m_sName)
{
  m_vspInstructions.clear();
  for (const auto& [time, spInstr] : other.m_vspInstructions)
  {
    m_vspInstructions.push_back(
        sequence::tTimedInstruction{time, spInstr->Clone()});
  }
}
std::shared_ptr<SSequenceLayer> SSequenceLayer::Clone()
{
  std::shared_ptr<SSequenceLayer> spRet = std::make_shared<SSequenceLayer>(*this);
  spRet->m_vspInstructions.clear();
  for (const auto& [pos, isntr] : m_vspInstructions)
  {
    spRet->m_vspInstructions.push_back({pos, isntr->Clone()});
  }
  return spRet;
}
QJsonObject SSequenceLayer::ToJsonObject()
{
  QJsonArray instructions;
  for (const sequence::tTimedInstruction& sMountPoint : m_vspInstructions)
  {
    QJsonObject obj = sMountPoint.second->ToJsonObject();
    QJsonObject objIn = {
      { "iTimePos", double(sMountPoint.first) }
    };
    objIn["instruction"] = QJsonValue(obj);
    instructions.push_back(objIn);
  }
  return {
    { "sLayerType", m_sLayerType },
    { "sName", m_sName },
    { "vspInstructions", instructions }
  };
}
void SSequenceLayer::FromJsonObject(const QJsonObject& json)
{
  m_vspInstructions.clear();
  auto it = json.find("sLayerType");
  if (it != json.end())
  {
    m_sLayerType = it.value().toString();
  }
  it = json.find("sName");
  if (it != json.end())
  {
    m_sName = it.value().toString();
  }
  it = json.find("vspInstructions");
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      sequence::tTimedInstruction instr;
      QJsonObject obj = val.toObject();
      it = obj.find("iTimePos");
      if (it != obj.end())
      {
        instr.first = static_cast<qint64>(it.value().toDouble());
      }
      it = obj.find("instruction");
      if (it != obj.end())
      {
        QJsonObject objInstr = it.value().toObject();
        it = objInstr.find("sInstructionType");
        if (it != objInstr.end())
        {
          std::shared_ptr<SSequenceInstruction> spInstruction =
              sequence::CreateInstruction(it.value().toString(), {});
          spInstruction->FromJsonObject(obj);
          instr.second = spInstruction;
        }
      }
      m_vspInstructions.push_back(instr);
    }
  }
}

//--------------------------------------------------------------------------------------
//
QJsonObject SSequenceFile::ToJsonObject()
{
  QJsonArray layers;
  for (const std::shared_ptr<SSequenceLayer>& spLayer : m_vspLayers)
  {
    layers.push_back(spLayer->ToJsonObject());
  }
  return {
    { "vspLayers", layers },
    { "iLengthMili", double(m_iLengthMili) }
  };
}
void SSequenceFile::FromJsonObject(const QJsonObject& json)
{
  m_vspLayers.clear();
  auto it = json.find("vspLayers");
  if (it != json.end())
  {
    for (QJsonValue val : it.value().toArray())
    {
      std::shared_ptr<SSequenceLayer> spLayer = std::make_shared<SSequenceLayer>();
      spLayer->FromJsonObject(val.toObject());
      m_vspLayers.push_back(spLayer);
    }
  }
  it = json.find("iLengthMili");
  if (it != json.end())
  {
    m_iLengthMili = it.value().toDouble();
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<SSequenceInstruction> sequence::CreateInstruction(const QString& sId,
                                                                  const QVariantList& vArgs)
{
  std::shared_ptr<SSequenceInstruction> ret = nullptr;
  auto it = GetFactoryFunctionMap().find(sId);
  if (GetFactoryFunctionMap().end() != it)
  {
    ret = it->second(vArgs);
  }
  return ret;
}

//----------------------------------------------------------------------------------------
//
qint64 sequence::TimeOffsetFromInstructionType(const QString& sId)
{
  if (c_sInstructionIdBeat == sId ||
      c_sInstructionIdStartPattern == sId)
  {
    return -1000;
  }
  return 0;
}
