#include "SequencePropertiesOverlay.h"
#include "CommandChangeSequenceProperties.h"
#include "ui_SequencePropertiesOverlay.h"

#include "Systems/Sequence/Sequence.h"

CSequencePropertiesOverlay::CSequencePropertiesOverlay(QWidget* pParent) :
  COverlayBase(0, pParent),
  m_spUi(std::make_unique<Ui::CSequencePropertiesOverlay>())
{
  m_spUi->setupUi(this);
  m_preferredSize = size();
}

CSequencePropertiesOverlay::~CSequencePropertiesOverlay()
{

}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::UpdateUi()
{
  if (nullptr != m_spCurrentSequence)
  {
    QTime time(0, 0);
    time = time.addMSecs(m_spCurrentSequence->m_iLengthMili);
    QSignalBlocker b(m_spUi->pTimeEdit);
    m_spUi->pTimeEdit->setTime(time);
  }
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::SetSequence(const tspSequence& spSeq)
{
  m_spCurrentSequence = spSeq;
  UpdateUi();
}

//----------------------------------------------------------------------------------------
//
tspSequence CSequencePropertiesOverlay::Sequence() const
{
  return m_spCurrentSequence;
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::SetSequenceName(const QString& sName)
{
  m_sCurrentSequenceName = sName;
}

//----------------------------------------------------------------------------------------
//
const QString& CSequencePropertiesOverlay::SequenceName() const
{
  return m_sCurrentSequenceName;
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::SetUndoStack(QPointer<QUndoStack> pUndo)
{
  m_pUndoStack = pUndo;
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::Climb()
{
  ClimbToFirstInstanceOf("CEditorPatternEditorWidget", false);
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::Resize()
{
  QSize newSize = m_preferredSize;
  if (m_pTargetWidget->geometry().width() < m_preferredSize.width())
  {
    newSize.setWidth(m_pTargetWidget->geometry().width());
  }
  if (m_pTargetWidget->geometry().height() < m_preferredSize.height())
  {
    newSize.setHeight(m_pTargetWidget->geometry().height());
  }

  QPoint newPos =
      QPoint(m_pTargetWidget->geometry().width() / 2, m_pTargetWidget->geometry().height() / 2) -
      QPoint(newSize.width() / 2, newSize.height() / 2);

  move(newPos.x(), newPos.y());
  resize(newSize);
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::on_pTimeEdit_timeChanged(const QTime&)
{
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::on_pConfirmButton_clicked()
{
  if (nullptr != m_pUndoStack && nullptr != m_spCurrentSequence)
  {
    m_pUndoStack->push(new CCommandChangeSequenceProperties(this, m_spUi->pTimeEdit));
  }

  Hide();
}

//----------------------------------------------------------------------------------------
//
void CSequencePropertiesOverlay::on_pCancelButton_clicked()
{
  Hide();
}
