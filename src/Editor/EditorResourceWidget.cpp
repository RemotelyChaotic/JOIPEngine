#include "EditorResourceWidget.h"
#include "Application.h"
#include "ResourceTreeItemModel.h"
#include "Backend/DatabaseManager.h"
#include "Backend/Project.h"
#include "ui_EditorResourceWidget.h"

#include <QFileDialog>
#include <QImageReader>
#include <QFileInfo>

CEditorResourceWidget::CEditorResourceWidget(QWidget* pParent) :
  QWidget(pParent),
  m_spUi(new Ui::CEditorResourceWidget),
  m_spCurrentProject(nullptr),
  m_bInitialized(false)
{
  m_spUi->setupUi(this);
  Initialize();
}

CEditorResourceWidget::~CEditorResourceWidget()
{

}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::Initialize()
{
  m_bInitialized = false;

  m_wpDbManager = CApplication::Instance()->System<CDatabaseManager>();

  CResourceTreeItemModel* pModel = new CResourceTreeItemModel(m_spUi->pResourceTree);
  m_spUi->pResourceTree->setModel(pModel);

  m_bInitialized = true;
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::LoadProject(tspProject spCurrentProject)
{
  if (!m_bInitialized) { return; }

  m_spCurrentProject = spCurrentProject;

  if (nullptr != m_spCurrentProject)
  {
    // load resource-tree
    CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());
    pModel->InitializeModel(m_spCurrentProject);
  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::UnloadProject()
{
  if (!m_bInitialized) { return; }

  m_spCurrentProject = nullptr;

  CResourceTreeItemModel* pModel = dynamic_cast<CResourceTreeItemModel*>(m_spUi->pResourceTree->model());
  pModel->DeInitializeModel();
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pFilterLineEdit_editingFinished()
{
  if (!m_bInitialized) { return; }
  // TODO: implement properly
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pAddButton_clicked()
{
  if (!m_bInitialized) { return; }

  QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
  QStringList imageFormatsList;
  QString sImageFormats;
  for (QByteArray arr : imageFormats) { imageFormatsList += "*." + QString::fromUtf8(arr); }
  sImageFormats = imageFormatsList.join(" ");

  // TODO: check video codecs
  QStringList videoFormatsList = QStringList() << "*.mp4" << "*.avi" << "*.mov";
  QString sVideoFormats = videoFormatsList.join(" ");

  // TODO: check devices
  QStringList audioFormatsList = QStringList() << "*.mp3" << "*.ogg";
  QString sAudioFormats = audioFormatsList.join(" ");

  QStringList otherFormatsList = QStringList() << "*.json" << "*.js";
  QString sOtherFormats = otherFormatsList.join(" ");


  QString sFormatSelection = "Image Files (%1);;Video Files (%2);;Sound Files (%3);;Other Files (%4)";
  QString sCurrentFolder = CApplication::Instance()->Settings()->ContentFolder();
  QString sFileName = QFileDialog::getOpenFileName(this,
      tr("Add File"), sCurrentFolder,
      sFormatSelection.arg(sImageFormats).arg(sVideoFormats).arg(sAudioFormats).arg(sOtherFormats));

  // add file to respective category
  QFileInfo info(sFileName);
  const QString sEnding = "*." + info.suffix();
  if (imageFormatsList.contains(sEnding))
  {

  }
  else if (videoFormatsList.contains(sEnding))
  {

  }
  else if (audioFormatsList.contains(sEnding))
  {

  }
  else if (otherFormatsList.contains(sEnding))
  {

  }
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pAddWebButton_clicked()
{
  // TODO: implement
}

//----------------------------------------------------------------------------------------
//
void CEditorResourceWidget::on_pRemoveButton_clicked()
{
  if (!m_bInitialized) { return; }
}
