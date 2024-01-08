#include "TimelineWidget.h"
#include "TimelineWidgetLayer.h"
#include "ui_TimelineWidget.h"

CTimelineWidget::CTimelineWidget(QWidget* pParent) :
  QScrollArea(pParent),
  m_spUi(std::make_unique<Ui::CTimelineWidget>())
{
  m_spUi->setupUi(this);
}

CTimelineWidget::~CTimelineWidget()
{
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddNewLayer()
{

}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::AddNewElement(const QString& sId)
{
  Q_UNUSED(sId)
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Clear()
{

}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::RemoveSelectedLayer()
{

}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetSequence(const tspSequence& spSeq)
{
  m_spCurrentSequence = spSeq;
  Update();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::SetUndoStack(QPointer<QUndoStack> pUndo)
{
  m_pUndoStack = pUndo;
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::Update()
{

}

//----------------------------------------------------------------------------------------
//
tspSequence CTimelineWidget::Sequence() const
{
  return m_spCurrentSequence;
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::minimumSizeHint() const
{
  return QSize(1,1);
}

//----------------------------------------------------------------------------------------
//
QSize CTimelineWidget::sizeHint() const
{
  return QScrollArea::sizeHint();
}

//----------------------------------------------------------------------------------------
//
void CTimelineWidget::paintEvent(QPaintEvent* pEvent)
{

}
