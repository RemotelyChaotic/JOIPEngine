#include "SequenceTextBoxRunner.h"

#include "Systems/Script/ScriptTextBox.h"

#include <QTextDocumentFragment>

namespace
{
  const qint32 c_iDelayBaseMs = 1'500;
  const qint32 c_iDelayPerCharMs = 30;
  const qint32 c_iDelayPerCharMaxMs = 8'000;
  const qint32 c_iDelayPerWordMs = 300;

  qint64 EstimateDurationBasedOnText(const QString& sText)
  {
    QString sPlainText = QTextDocumentFragment::fromHtml(sText).toPlainText();
    return (
        c_iDelayBaseMs +
        std::max(
            std::min(sPlainText.size() * c_iDelayPerCharMs, c_iDelayPerCharMaxMs),
            (sPlainText.count(QRegExp("\\s")) + 1) * c_iDelayPerWordMs
            )
        );
  }
}

//----------------------------------------------------------------------------------------
//
CSequenceTextBoxRunner::CSequenceTextBoxRunner(
    std::weak_ptr<CScriptCommunicator> pCommunicator) :
  CScriptObjectBase(pCommunicator),
  ISequenceObjectRunner()
{
}
CSequenceTextBoxRunner::~CSequenceTextBoxRunner()
{

}

//----------------------------------------------------------------------------------------
//
void CSequenceTextBoxRunner::RunSequenceInstruction(const QString&,
                                                    const std::shared_ptr<SSequenceInstruction>& spInstr,
                                                    const SProjectData&)
{
  if (auto spComm = m_wpCommunicator.lock())
  {
    if (auto spSignalEmitter = spComm->LockedEmitter<CTextBoxSignalEmitter>())
    {
      if (const auto& spI = std::dynamic_pointer_cast<SShowTextInstruction>(spInstr);
          nullptr != spI)
      {
        if (spI->m_textColor.has_value())
        {
          emit spSignalEmitter->textColorsChanged({spI->m_textColor.value()});
        }
        if (spI->m_bgColor.has_value())
        {
          emit spSignalEmitter->textBackgroundColorsChanged({spI->m_bgColor.value()});
        }
        if (spI->m_sPortrait.has_value())
        {
          emit spSignalEmitter->textPortraitChanged(spI->m_sPortrait.value());
        }
        if (spI->m_sText.has_value())
        {
          double dWaitTime = 0.0;
          bool bSkipable = false;
          if (0 > dWaitTime)
          {
            dWaitTime = static_cast<double>(EstimateDurationBasedOnText(spI->m_sText.value())) / 1000.0;
          }

          emit spSignalEmitter->showText(spI->m_sText.value(), bSkipable ? dWaitTime : 0, QString());
        }
      }
    }
  }
}
