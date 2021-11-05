#ifndef CJSONINSTRUCTIONSETRUNNER_H
#define CJSONINSTRUCTIONSETRUNNER_H

#include <QString>
#include <QVariant>
#include <memory>
#include <variant>

class CJsonInstructionSetRunnerPrivate;

struct SJsonException
{
  QString m_sException;
  QString m_sToken;
  QString m_sCommand;
  qint32  m_iLineNr;
  qint32  m_iColumn;
};

enum class ERunerMode
{
  eAutoRunAll,  // run all commands blocking
  eRunOne       // return after each command and advance runner state
};

//----------------------------------------------------------------------------------------
//
enum class ENextCommandToCall
{
  eChild = 0,
  eSibling
};

template<ENextCommandToCall M> struct SRunRetVal : std::false_type {};
template<> struct SRunRetVal<ENextCommandToCall::eChild>
{
  SRunRetVal(qint32 iIndex) : m_iIndex(iIndex) {}
  ENextCommandToCall m_type = ENextCommandToCall::eChild;
  qint32             m_iIndex;
};
template<> struct SRunRetVal<ENextCommandToCall::eSibling>
{
  SRunRetVal() {}
  ENextCommandToCall m_type = ENextCommandToCall::eSibling;
};

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetRunner
{
  friend class CJsonInstructionSetParserPrivate;

public:
  typedef std::variant<SJsonException, bool /*Has more commands*/> tRetVal;

  explicit CJsonInstructionSetRunner();
  ~CJsonInstructionSetRunner();

  tRetVal CallNextCommand(ERunerMode runMode = ERunerMode::eRunOne);
  tRetVal Run(const QString& sInstructionSet, ERunerMode runMode = ERunerMode::eRunOne);

protected:
  std::unique_ptr<CJsonInstructionSetRunnerPrivate> m_pPrivate;
};

#endif // CJSONINSTRUCTIONSETRUNNER_H
