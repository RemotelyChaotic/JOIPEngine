#ifndef CTIMELINESEQEUNCEINSTRUCTIONWIDGET_H
#define CTIMELINESEQEUNCEINSTRUCTIONWIDGET_H

#include "Systems/Sequence/Sequence.h"

#include <QWidget>

#include <memory>

class CResourceTreeItemModel;
class CTimelineSeqeunceInstructionWidgetBase;
namespace Ui {
  class CTimelineSeqeunceInstructionWidgetEval;
  class CTimelineSeqeunceInstructionWidgetLinearToy;
  class CTimelineSeqeunceInstructionWidgetPauseAudio;
  class CTimelineSeqeunceInstructionWidgetPlayAudio;
  class CTimelineSeqeunceInstructionWidgetPlayVideo;
  class CTimelineSeqeunceInstructionWidgetRotateToy;
  class CTimelineSeqeunceInstructionWidgetRunScript;
  class CTimelineSeqeunceInstructionWidgetShowMedia;
  class CTimelineSeqeunceInstructionWidgetShowText;
  class CTimelineSeqeunceInstructionWidgetSingleBeat;
  class CTimelineSeqeunceInstructionWidgetStartPattern;
  class CTimelineSeqeunceInstructionWidgetStopAudio;
  class CTimelineSeqeunceInstructionWidgetVibrate;
}

namespace sequence
{
  CTimelineSeqeunceInstructionWidgetBase* CreateWidgetFromInstruction(
      const std::shared_ptr<SSequenceInstruction>& spInstr, QWidget* pParent);
}

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetBase : public QWidget
{
  Q_OBJECT
public:
  explicit CTimelineSeqeunceInstructionWidgetBase(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetBase() override;

  bool IsForcedOpen() const { return m_bForcedOpen; }
  virtual bool HasUi() const { return true; }

  virtual void SetResourceModel(CResourceTreeItemModel*) {  }
  virtual void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) = 0;
  virtual std::shared_ptr<SSequenceInstruction> Properties() const = 0;

signals:
  void SignalChangedProperties();

protected slots:
  void EmitPropertiesChanged();

protected:
  QString m_sType;
  bool m_bIsInitializing;
  bool m_bForcedOpen = false;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetSingleBeat : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetSingleBeat(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetSingleBeat() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected:
  void on_pEnabledCheckBox_toggled(bool bOn);

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetSingleBeat> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetStartPattern : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetStartPattern(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetStartPattern() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pVolumeCheckBox_toggled(bool bOn);
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);
  void on_AddButton_clicked();
  void on_RemoveButton_clicked();
  void on_AddPatternElemButton_clicked();
  void on_RemovePatternElemButton_clicked();

protected:
  void AddPatternElement(double dValue);
  void AddResourceElement(const QString& sElem);
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetStartPattern> m_spUi;
  qint32                                                              m_iEditingRow = -1;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetVibrate : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetVibrate(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetVibrate() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetVibrate> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetLinearToy : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetLinearToy(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetLinearToy() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetLinearToy> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetRotateToy : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetRotateToy(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetRotateToy() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetRotateToy> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetShowMedia : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetShowMedia(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetShowMedia() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetShowMedia> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetPlayVideo : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetPlayVideo(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetPlayVideo() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pLoopsCheckBox_toggled(bool bEnabled);
  void on_pStartAtCheckBox_toggled(bool bEnabled);
  void on_pEndAtCheckBox_toggled(bool bEnabled);
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetPlayVideo> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetPlayAudio : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetPlayAudio(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetPlayAudio() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pLoopsCheckBox_toggled(bool bEnabled);
  void on_pStartAtCheckBox_toggled(bool bEnabled);
  void on_pEndAtCheckBox_toggled(bool bEnabled);
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetPlayAudio> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetPauseAudio : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetPauseAudio(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetPauseAudio() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetPauseAudio> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetStopAudio : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetStopAudio(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetStopAudio() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetStopAudio> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetShowText : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetShowText(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetShowText() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pIconCheckBox_toggled(bool bEnabled);
  void on_pSetSleepTimeCheckBox_toggled(bool bEnabled);
  void on_pAutoTimeCheckBox_toggled(bool bEnabled);
  void on_pEnableTextColorCheckBox_toggled(bool bEnabled);
  void on_pEnableBgColorCheckBox_toggled(bool bEnabled);
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetShowText> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetRunScript : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetRunScript(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetRunScript() override;

  void SetResourceModel(CResourceTreeItemModel* pResourceModel) override;
  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

protected slots:
  void on_pResourceSelectTree_doubleClicked(const QModelIndex& index);

protected:
  bool eventFilter(QObject* pObj, QEvent* pEvt) override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetRunScript> m_spUi;
};

//----------------------------------------------------------------------------------------
//
class CTimelineSeqeunceInstructionWidgetEval : public CTimelineSeqeunceInstructionWidgetBase
{
  Q_OBJECT

public:
  explicit CTimelineSeqeunceInstructionWidgetEval(QWidget* pParent = nullptr);
  ~CTimelineSeqeunceInstructionWidgetEval() override;

  void SetProperties(const std::shared_ptr<SSequenceInstruction>& spInstr) override;
  std::shared_ptr<SSequenceInstruction> Properties() const override;

private:
  std::unique_ptr<Ui::CTimelineSeqeunceInstructionWidgetEval> m_spUi;
};

#endif // CTIMELINESEQEUNCEINSTRUCTIONWIDGET_H
