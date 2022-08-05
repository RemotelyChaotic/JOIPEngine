#ifndef EOSCOMMANDMODELS_H
#define EOSCOMMANDMODELS_H

#include "Systems/EOS/CommandEosAudioBase.h"
#include "Systems/EOS/CommandEosChoiceBase.h"
#include "Systems/EOS/CommandEosDisableSceneBase.h"
#include "Systems/EOS/CommandEosEnableSceneBase.h"
#include "Systems/EOS/CommandEosEndBase.h"
#include "Systems/EOS/CommandEosEvalBase.h"
#include "Systems/EOS/CommandEosGotoBase.h"
#include "Systems/EOS/CommandEosIfBase.h"
#include "Systems/EOS/CommandEosImageBase.h"
#include "Systems/EOS/CommandEosNoopBase.h"
#include "Systems/EOS/CommandEosNotificationCloseBase.h"
#include "Systems/EOS/CommandEosNotificationCreateBase.h"
#include "Systems/EOS/CommandEosPromptBase.h"
#include "Systems/EOS/CommandEosSayBase.h"
#include "Systems/EOS/CommandEosTimerBase.h"
#include "Systems/EOS/EosCommands.h"


class IEosCommandModel
{
public:
  IEosCommandModel() {}
  virtual ~IEosCommandModel() {}

  virtual tInstructionMapValue DefaultArgs() = 0;
  virtual QString DisplayName(const tInstructionMapValue& args) = 0;
  virtual void InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                               const QString& sIntoGroup, const QString& sInsertedChild) = 0;
  virtual void RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                             const QString& sIntoGroup) = 0;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosAudioModel : public CCommandEosAudioBase, public IEosCommandModel
{
public:
  CCommandEosAudioModel();
  ~CCommandEosAudioModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosChoiceModel : public CCommandEosChoiceBase, public IEosCommandModel
{
public:
  CCommandEosChoiceModel();
  ~CCommandEosChoiceModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                       const QString& sIntoGroup, const QString& sInsertedChild) override;
  void RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                     const QString& sIntoGroup) override;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosDisableSceneModel : public CCommandEosDisableSceneBase, public IEosCommandModel
{
public:
  CCommandEosDisableSceneModel();
  ~CCommandEosDisableSceneModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosEnableSceneModel : public CCommandEosEnableSceneBase, public IEosCommandModel
{
public:
  CCommandEosEnableSceneModel();
  ~CCommandEosEnableSceneModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosEndModel : public CCommandEosEndBase, public IEosCommandModel
{
public:
  CCommandEosEndModel();
  ~CCommandEosEndModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosEvalModel : public CCommandEosEvalBase, public IEosCommandModel
{
public:
  CCommandEosEvalModel();
  ~CCommandEosEvalModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosGotoModel : public CCommandEosGotoBase, public IEosCommandModel
{
 public:
  CCommandEosGotoModel();
  ~CCommandEosGotoModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosIfModel : public CCommandEosIfBase, public IEosCommandModel
{
public:
  CCommandEosIfModel();
  ~CCommandEosIfModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                       const QString& sIntoGroup, const QString& sInsertedChild) override;
  void RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                     const QString& sIntoGroup) override;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosImageModel : public CCommandEosImageBase, public IEosCommandModel
{
public:
  CCommandEosImageModel();
  ~CCommandEosImageModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNoopModel : public CCommandEosNoopBase, public IEosCommandModel
{
public:
  CCommandEosNoopModel();
  ~CCommandEosNoopModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationCreateModel : public CCommandEosNotificationCreateBase, public IEosCommandModel
{
public:
  CCommandEosNotificationCreateModel();
  ~CCommandEosNotificationCreateModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                       const QString& sIntoGroup, const QString& sInsertedChild) override;
  void RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                     const QString& sIntoGroup) override;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosNotificationCloseModel : public CCommandEosNotificationCloseBase, public IEosCommandModel
{
public:
  CCommandEosNotificationCloseModel();
  ~CCommandEosNotificationCloseModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosPromptModel : public CCommandEosPromptBase, public IEosCommandModel
{
public:
  CCommandEosPromptModel();
  ~CCommandEosPromptModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosSayModel : public CCommandEosSayBase, public IEosCommandModel
{
public:
  CCommandEosSayModel();
  ~CCommandEosSayModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue*, qint32, const QString&, const QString&) override {}
  void RemoveChildAt(tInstructionMapValue*, qint32,const QString&) override {}
};

//----------------------------------------------------------------------------------------
//
class CCommandEosTimerModel : public CCommandEosTimerBase, public IEosCommandModel
{
public:
  CCommandEosTimerModel();
  ~CCommandEosTimerModel() override;

  tInstructionMapValue DefaultArgs() override;
  QString DisplayName(const tInstructionMapValue& args) override;
  void InsertedChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                       const QString& sIntoGroup, const QString& sInsertedChild) override;
  void RemoveChildAt(tInstructionMapValue* pArgs, qint32 iIndex,
                     const QString& sIntoGroup) override;
};


#endif // EOSCOMMANDMODELS_H
