#include "SequencePreviewWidget.h"
#include "ui_SequencePreviewWidget.h"

CSequencePreviewWidget::CSequencePreviewWidget(QWidget* pParent) :
    QWidget(pParent),
    m_spUi(std::make_unique<Ui::CSequencePreviewWidget>())
{
  m_spUi->setupUi(this);
}

CSequencePreviewWidget::~CSequencePreviewWidget()
{
}
