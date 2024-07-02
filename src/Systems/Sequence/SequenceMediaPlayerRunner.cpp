#include "SequenceMediaPlayerRunner.h"

#include "Systems/Script/ScriptMediaPlayer.h"

using namespace std::placeholders;

CSequenceMediaPlayerRunner::CSequenceMediaPlayerRunner(
    QPointer<CScriptRunnerSignalEmiter> pEmitter) :
  CScriptObjectBase(pEmitter),
  ISequenceObjectRunner(),
  m_functionMap({{sequence::c_sInstructionIdShow, std::bind(&CSequenceMediaPlayerRunner::RunShow, this, _1)},
                 {sequence::c_sInstructionIdPlayVideo, std::bind(&CSequenceMediaPlayerRunner::RunPlayVideo, this, _1)},
                 {sequence::c_sInstructionIdPauseVideo, std::bind(&CSequenceMediaPlayerRunner::RunPauseVideo, this, _1)},
                 {sequence::c_sInstructionIdStopVideo, std::bind(&CSequenceMediaPlayerRunner::RunStopVideo, this, _1)},
                 {sequence::c_sInstructionIdPlayAudio, std::bind(&CSequenceMediaPlayerRunner::RunPlayAudio, this, _1)},
                 {sequence::c_sInstructionIdPauseAudio, std::bind(&CSequenceMediaPlayerRunner::RunPauseAudio, this, _1)},
                 {sequence::c_sInstructionIdStopAudio, std::bind(&CSequenceMediaPlayerRunner::RunStopAudio, this, _1)}})
{
}
CSequenceMediaPlayerRunner::~CSequenceMediaPlayerRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunSequenceInstruction(const QString&,
                                                        const std::shared_ptr<SSequenceInstruction>& spInstr,
                                                        const SProjectData&)
{
  auto it = m_functionMap.find(spInstr->m_sInstructionType);
  if (m_functionMap.end() != it)
  {
    it->second(spInstr);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunShow(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SShowMediaInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->playMedia(spI->m_sResource, 1, 0, -1);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunPlayVideo(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SPlayVideoInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->playMedia(spI->m_sResource, spI->m_iLoops.value_or(1),
                                   spI->m_iStartAt.value_or(0), spI->m_iEndAt.value_or(-1));
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunPauseVideo(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SPauseVideoInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->pauseVideo();
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunStopVideo(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SStopVideoInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->stopVideo();
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunPlayAudio(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SPlayAudioInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->playSound(spI->m_sResource, spI->m_sName.value_or(QString()),
                                   spI->m_iLoops.value_or(1), spI->m_iStartAt.value_or(0),
                                   spI->m_iEndAt.value_or(-1));
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunPauseAudio(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SPauseAudioInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->pauseSound(spI->m_sName);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMediaPlayerRunner::RunStopAudio(const std::shared_ptr<SSequenceInstruction>& spInstr)
{
  auto pSignalEmitter = SignalEmitter<CMediaPlayerSignalEmitter>();
  if (const auto& spI = std::dynamic_pointer_cast<SStopAudioInstruction>(spInstr);
      nullptr != spI && nullptr != pSignalEmitter)
  {
    emit pSignalEmitter->stopSound(spI->m_sName);
  }
}
