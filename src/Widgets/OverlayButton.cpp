#include "OverlayButton.h"
#include "Application.h"
#include "Settings.h"
#include "Widgets/ShortcutButton.h"

#include <QVBoxLayout>

qint32 COverlayButton::c_iOverlayButtonZOrder = 0;

//----------------------------------------------------------------------------------------
//
COverlayButton::COverlayButton(const QString& sOverlayName,
                               const QString& sButtonName,
                               const QString& sToolTip,
                               const QString& sKeyBinding,
                               QWidget* pParent) :
  COverlayBase(c_iOverlayButtonZOrder, pParent),
  m_sOverlayName(sOverlayName),
  m_sButtonName(sButtonName),
  m_sToolTip(sToolTip),
  m_sKeyBinding(sKeyBinding)
{
  setObjectName(m_sOverlayName);
  QSizePolicy sizePolicyThis(QSizePolicy::Preferred, QSizePolicy::Fixed);
  sizePolicyThis.setHorizontalStretch(0);
  sizePolicyThis.setVerticalStretch(0);
  sizePolicyThis.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicyThis);
  setMinimumSize(QSize(0, 48));
  setMaximumSize(QSize(16777215, 48));
  setAttribute(Qt::WA_TranslucentBackground);

  QVBoxLayout* pLayout = new QVBoxLayout(this);
  pLayout->setSpacing(0);
  pLayout->setContentsMargins(0, 0, 0, 0);

  m_pButton = new CShortcutButton(this);
  m_pButton->setObjectName(m_sButtonName);
  QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy2.setHorizontalStretch(48);
  sizePolicy2.setVerticalStretch(48);
  sizePolicy2.setHeightForWidth(m_pButton->sizePolicy().hasHeightForWidth());
  m_pButton->setSizePolicy(sizePolicy2);
  m_pButton->setMinimumSize(QSize(48, 48));
  m_pButton->setMaximumSize(QSize(48, 48));
  m_pButton->setIconSize(QSize(48, 48));
  m_pButton->EnableShortcutAsText(true);

  auto spSettings = CApplication::Instance()->Settings();
  connect(spSettings.get(), &CSettings::keyBindingsChanged,
          this, &COverlayButton::SlotKeyBindingsChanged);
  m_pButton->SetShortcut(spSettings->keyBinding(m_sKeyBinding));

  pLayout->addWidget(m_pButton);

  retranslateUi(this);
}

COverlayButton::~COverlayButton()
{

}

//----------------------------------------------------------------------------------------
//
void COverlayButton::Initialize()
{
  connect(m_pButton, &QPushButton::clicked,
          this, &COverlayButton::SignalButtonClicked, Qt::UniqueConnection);
}

//----------------------------------------------------------------------------------------
//
void COverlayButton::Climb()
{
  ClimbToTop();
}

//----------------------------------------------------------------------------------------
//
void COverlayButton::Resize()
{
}

//----------------------------------------------------------------------------------------
//
void COverlayButton::SlotKeyBindingsChanged()
{
  auto spSettings = CApplication::Instance()->Settings();
  m_pButton->SetShortcut(spSettings->keyBinding(m_sKeyBinding));
}

//----------------------------------------------------------------------------------------
//
void COverlayButton::retranslateUi(QWidget* pHelpOverlay)
{
  Q_UNUSED(pHelpOverlay)
  m_pButton->setToolTip(QApplication::translate("COverlayButton", m_sToolTip.toStdString().data(), Q_NULLPTR));
}
