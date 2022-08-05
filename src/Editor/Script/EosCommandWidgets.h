#ifndef CEOSCOMMANDWIDGETAUDIO_H
#define CEOSCOMMANDWIDGETAUDIO_H

#include "Systems/JSON/JsonInstructionTypes.h"

#include <QPointer>
#include <QWidget>
#include <memory>

class CEosCommandWidgetBase;
class CResourceTreeItemModel;
class QCompleter;
class QRegularExpressionValidator;
class QStandardItemModel;
class QTableWidgetItem;
namespace Ui {
  class CEosCommandWidgetAudio;
  class CEosCommandWidgetChoice;
  class CEosCommandWidgetEval;
  class CEosCommandWidgetGoto;
  class CEosCommandWidgetIf;
  class CEosCommandWidgetImage;
  class CEosCommandWidgetNotificationClose;
  class CEosCommandWidgetNotificationCreate;
  class CEosCommandWidgetPrompt;
  class CEosCommandWidgetSay;
  class CEosCommandWidgetScene;
  class CEosCommandWidgetTimer;
}

namespace eos
{
  CEosCommandWidgetBase* CreateWidgetFromType(const QString& sType, QWidget* pParent);
}

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetBase : public QWidget
{
  Q_OBJECT
public:
  explicit CEosCommandWidgetBase(QWidget* pParent = nullptr);
  ~CEosCommandWidgetBase();

  bool IsForcedOpen() const { return m_bForcedOpen; }
  virtual bool HasUi() const { return true; }

  virtual void SetProperties(tInstructionMapValue* pProps) = 0;
  virtual void SetResourceModel(CResourceTreeItemModel*) {  }

signals:
  void SignalChangedProperties();
  void SignalInvalidateItemChildren();

protected:
  bool m_bIsInitializing;
  bool m_bForcedOpen = false;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetAudio : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetAudio(QWidget* pParent = nullptr);
  ~CEosCommandWidgetAudio();

  void SetProperties(tInstructionMapValue* pProps) override;
  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;

protected slots:
  void on_pLocatorLineEdit_editingFinished();
  void on_SearchButton_clicked();
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);
  void on_pIdLineEdit_editingFinished();
  void on_pLoopsSpinBox_valueChanged(qint64 iValue);
  void on_pSeekSpinBox_valueChanged(qint64 iValue);
  void on_pStartSpinBox_valueChanged(qint64 iValue);
  void on_pVolumeSlider_valueChanged(qint32 iValue);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CEosCommandWidgetAudio> m_spUi;
  tInstructionMapValue*                       m_pProps = nullptr;
  QPointer<QRegularExpressionValidator>       m_pValidator = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetChoice : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetChoice(QWidget* pParent = nullptr);
  ~CEosCommandWidgetChoice();

  void SetProperties(tInstructionMapValue* pProps) override;

protected:
  void on_AddButtonButton_clicked();
  void on_RemoveButtonButton_clicked();
  void on_pTableWidget_itemChanged(QTableWidgetItem* pItem);
  void SlotColorChanged(const QColor& color);

private:
  void AddItemToList(qint32 iRow, const QString& sText, const QColor& col, bool bVisible);

  std::unique_ptr<Ui::CEosCommandWidgetChoice> m_spUi;
  tInstructionMapValue*                        m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetEnd : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetEnd(QWidget* pParent = nullptr);
  ~CEosCommandWidgetEnd();

  bool HasUi() const override { return false; }

  void SetProperties(tInstructionMapValue* pProps) override;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetEval : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetEval(QWidget* pParent = nullptr);
  ~CEosCommandWidgetEval();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pPlainTextEdit_contentsChange();

private:
  std::unique_ptr<Ui::CEosCommandWidgetEval> m_spUi;
  tInstructionMapValue*                      m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetGoto : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetGoto(QWidget* pParent = nullptr);
  ~CEosCommandWidgetGoto();

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetGoto> m_spUi;
  QPointer<QCompleter>                       m_pCompleter;
  QPointer<QStandardItemModel>               m_pCompleterModel;
  tInstructionMapValue*                      m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetIf : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetIf(QWidget* pParent = nullptr);
  ~CEosCommandWidgetIf();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pConditionLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetIf> m_spUi;
  tInstructionMapValue*                    m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetImage : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetImage(QWidget* pParent = nullptr);
  ~CEosCommandWidgetImage();

  void SetProperties(tInstructionMapValue* pProps) override;
  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;

protected slots:
  void on_pResourceLineEdit_editingFinished();
  void on_SearchButton_clicked();
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CEosCommandWidgetImage> m_spUi;
  tInstructionMapValue*                       m_pProps = nullptr;
  QPointer<QRegularExpressionValidator>       m_pValidator = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetNotificationClose : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetNotificationClose(QWidget* pParent = nullptr);
  ~CEosCommandWidgetNotificationClose();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetNotificationClose> m_spUi;
  tInstructionMapValue*                                   m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetNotificationCreate : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetNotificationCreate(QWidget* pParent = nullptr);
  ~CEosCommandWidgetNotificationCreate();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pIdLineEdit_editingFinished();
  void on_pTitleLineEdit_editingFinished();
  void on_pButtonLineEdit_editingFinished();
  void on_pDurationLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetNotificationCreate> m_spUi;
  QPointer<QRegularExpressionValidator>                    m_pTimeValidator;
  tInstructionMapValue*                                    m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetPrompt : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetPrompt(QWidget* pParent = nullptr);
  ~CEosCommandWidgetPrompt();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetPrompt> m_spUi;
  tInstructionMapValue*                        m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetSay : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetSay(QWidget* pParent = nullptr);
  ~CEosCommandWidgetSay();

  void SetProperties(tInstructionMapValue* pProps) override;

protected slots:
  void on_pLabelPlainTextEdit_contentsChange();
  void on_pAlignComboBox_currentIndexChanged(qint32 iIndex);
  void on_pModeComboBox_currentIndexChanged(qint32 iIndex);
  void on_pAllowSkipCheckBox_toggled(bool bValue);
  void on_pDurationLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetSay> m_spUi;
  QPointer<QRegularExpressionValidator>     m_pTimeValidator;
  tInstructionMapValue*                     m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetScene : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetScene(QWidget* pParent = nullptr);
  ~CEosCommandWidgetScene();

  void SetProperties(tInstructionMapValue* pProps) override;
  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;

protected:
  void on_pSceneLineEdit_editingFinished();

private:
  std::unique_ptr<Ui::CEosCommandWidgetScene> m_spUi;
  QPointer<QCompleter>                        m_pCompleter;
  QPointer<QStandardItemModel>                m_pCompleterModel;
  tInstructionMapValue*                       m_pProps = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosCommandWidgetTimer : public CEosCommandWidgetBase
{
  Q_OBJECT

public:
  explicit CEosCommandWidgetTimer(QWidget* pParent = nullptr);
  ~CEosCommandWidgetTimer();

  void SetProperties(tInstructionMapValue* pProps) override;

protected:
  void on_pDurationLineEdit_editingFinished();
  void on_pAsynchCheckBox_toggled(bool bValue);
  void on_pStyleComboBox_currentIndexChanged(qint32 iIndex);

private:
  std::unique_ptr<Ui::CEosCommandWidgetTimer> m_spUi;
  QPointer<QRegularExpressionValidator>       m_pTimeValidator;
  tInstructionMapValue*                       m_pProps = nullptr;
};

#endif // CEOSCOMMANDWIDGETAUDIO_H
