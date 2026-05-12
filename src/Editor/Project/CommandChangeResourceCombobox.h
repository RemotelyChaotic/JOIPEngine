#ifndef CCOMMANDCHANGELAYOUT_H
#define CCOMMANDCHANGELAYOUT_H

#include <QComboBox>
#include <QPointer>
#include <QUndoCommand>
#include <memory>

struct SProject;
typedef std::shared_ptr<SProject>      tspProject;

class CCommandChangeResourceComboBox : public QUndoCommand
{
public:
  CCommandChangeResourceComboBox(QPointer<QComboBox> pComboBox,
                                 const tspProject& spCurrentProject,
                                 const std::function<void(void)>& fnOnUndoRedo,
                                 const QString& sText,
                                 QUndoCommand* pParent = nullptr);
  ~CCommandChangeResourceComboBox() override;

  void undo() override;
  void redo() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  virtual void SetNewValue(const QString& sValue) = 0;

  QPointer<QComboBox>     m_pComboBox;
  std::function<void(void)> m_fnOnUndoRedo;
  tspProject              m_spCurrentProject;
  QString                 m_sOriginalValue;
  QString                 m_sNewValue;
};

//----------------------------------------------------------------------------------------
//
class CCommandChangeLayout : public CCommandChangeResourceComboBox
{
public:
  CCommandChangeLayout(QPointer<QComboBox> pComboBox,
                       const tspProject& spCurrentProject,
                       const std::function<void(void)>& fnOnUndoRedo,
                       QUndoCommand* pParent = nullptr);
  ~CCommandChangeLayout() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void SetNewValue(const QString& sValue) override;
};

//----------------------------------------------------------------------------------------
//
class CCommandChangePreloadScript : public CCommandChangeResourceComboBox
{
public:
  CCommandChangePreloadScript(QPointer<QComboBox> pComboBox,
                              const tspProject& spCurrentProject,
                              const std::function<void(void)>& fnOnUndoRedo,
                              QUndoCommand* pParent = nullptr);
  ~CCommandChangePreloadScript() override;

  int id() const override;
  bool mergeWith(const QUndoCommand* pOther) override;

protected:
  void SetNewValue(const QString& sValue) override;
};

#endif // CCOMMANDCHANGELAYOUT_H
