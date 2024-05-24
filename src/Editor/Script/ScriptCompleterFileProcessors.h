#ifndef CSCRIPTCOMPLETERFILEPROCESSORJS_H
#define CSCRIPTCOMPLETERFILEPROCESSORJS_H

#include "ScriptEditorCompleterModel.h"

class CScriptCompleterFileProcessorJs : public IScriptCompleterFileProcessor
{
public:
  CScriptCompleterFileProcessorJs();
  ~CScriptCompleterFileProcessorJs();

  bool IsEndOfWordChar(QChar c) const override;
  void ProcessFile(int destRole, QTextStream& data, SCompleterModelData* pOutData) override;
  void ProcessLine(int destRole, const QString& sLine, SCompleterModelData* pOutData) override;

private:
  std::set<QString> m_vsInbuiltKeywords;
};

class CScriptCompleterFileProcessorLua : public IScriptCompleterFileProcessor
{
public:
  CScriptCompleterFileProcessorLua();
  ~CScriptCompleterFileProcessorLua();

  bool IsEndOfWordChar(QChar c) const override;
  void ProcessFile(int destRole, QTextStream& data, SCompleterModelData* pOutData) override;
  void ProcessLine(int destRole, const QString& sLine, SCompleterModelData* pOutData) override;

private:
  std::set<QString> m_vsInbuiltKeywords;
};

class CScriptCompleterFileProcessorQml : public IScriptCompleterFileProcessor
{
public:
  CScriptCompleterFileProcessorQml();
  ~CScriptCompleterFileProcessorQml();

  bool IsEndOfWordChar(QChar c) const override;
  void ProcessFile(int destRole, QTextStream& data, SCompleterModelData* pOutData) override;
  void ProcessLine(int destRole, const QString& sLine, SCompleterModelData* pOutData) override;

private:
  std::set<QString> m_vsInbuiltKeywords;
};

#endif // CSCRIPTCOMPLETERFILEPROCESSORJS_H
