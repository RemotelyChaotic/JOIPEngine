#include "SequenceMetronomeRunner.h"
#include "Application.h"

#include "Systems/MetronomeManager.h"
#include "Systems/Project.h"

#include "Systems/Script/ScriptMetronome.h"

using namespace std::placeholders;

CSequenceMetronomeRunner::CSequenceMetronomeRunner(
    std::weak_ptr<CScriptCommunicator> pCommunicator) :
  CScriptObjectBase(pCommunicator),
  ISequenceObjectRunner(),
  m_functionMap({{sequence::c_sInstructionIdBeat, std::bind(&CSequenceMetronomeRunner::RunSingleBeat, this, _1, _2, _3)},
                 {sequence::c_sInstructionIdStartPattern, std::bind(&CSequenceMetronomeRunner::RunStartPattern, this, _1, _2, _3)},
                 {sequence::c_sInstructionIdStopPattern, std::bind(&CSequenceMetronomeRunner::RunStopPattern, this, _1, _2, _3)}}),
  m_wpMetronomeManager(CApplication::Instance()->System<CMetronomeManager>())
{
}
CSequenceMetronomeRunner::~CSequenceMetronomeRunner()
{
}

//----------------------------------------------------------------------------------------
//
void CSequenceMetronomeRunner::RunSequenceInstruction(const QString& sName,
                                                      const std::shared_ptr<SSequenceInstruction>& spInstr,
                                                      const SProjectData& proj)
{
  auto it = m_functionMap.find(spInstr->m_sInstructionType);
  if (m_functionMap.end() != it)
  {
    it->second(sName, spInstr, proj);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMetronomeRunner::RunSingleBeat(const QString& sName,
                                             const std::shared_ptr<SSequenceInstruction>& spInstr,
                                             const SProjectData& proj)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SSingleBeatInstruction>(spInstr);
          nullptr != spI)
      {
        if (auto spMetronomeManager = m_wpMetronomeManager.lock())
        {
          CMetronomeManager::tId uid = spMetronomeManager->IdForName(sName);
          spMetronomeManager->SpawnSingleBeat(uid);
          ETickTypeFlags flags = ETickType::eSingle;
          {
            if (0 != (proj.m_metCmdMode & EToyMetronomeCommandModeFlag::eVibrate)) flags |= ETickType::eVibrateTick;
            if (0 != (proj.m_metCmdMode & EToyMetronomeCommandModeFlag::eLinear)) flags |= ETickType::eLinearTick;
            if (0 != (proj.m_metCmdMode & EToyMetronomeCommandModeFlag::eRotate)) flags |= ETickType::eRotateTick;
          }
          emit spMetronomeManager->SignalStart(uid, flags);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMetronomeRunner::RunStartPattern(const QString&,
                                               const std::shared_ptr<SSequenceInstruction>& spInstr,
                                               const SProjectData&)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SStartPatternInstruction>(spInstr);
          nullptr != spI)
      {
        QList<double> vdVals;
        for (double d : spI->m_vdPattern)
        {
          vdVals << d;
        }
        if (spI->m_dVolume.has_value())
        {
          emit spSignalEmitter->setVolume(spI->m_dVolume.value());
        }
        if (spI->m_vsResources.has_value())
        {
          emit spSignalEmitter->setBeatResource(spI->m_vsResources.value());
        }
        emit spSignalEmitter->setBpm(spI->m_iBpm);
        emit spSignalEmitter->setPattern(vdVals);
        emit spSignalEmitter->start();
      }
    }
  }
}

//----------------------------------------------------------------------------------------
//
void CSequenceMetronomeRunner::RunStopPattern(const QString&,
                                              const std::shared_ptr<SSequenceInstruction>& spInstr,
                                              const SProjectData&)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CMetronomeSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SStopPatternInstruction>(spInstr);
          nullptr != spI)
      {
        emit spSignalEmitter->stop();
      }
    }
  }
}
