#ifndef CCOMMONCODEGENERATOR_H
#define CCOMMONCODEGENERATOR_H

#include "ICodeGenerator.h"

struct SCommonCodeConfiguration
{
  QChar statementFinish;

  QChar arrStart;
  QChar arrEnd;

  QChar callStart;
  QChar callEnd;

  QChar stringChar;

  QChar invokationOp;
  QChar memberOp;

  QString sComment;

  QString sTrue;
  QString sFalse;
  QString sNull;

  QString sLocalKeyWord;
};

class CCommonCodeGenerator : public ICodeGenerator
{
public:
  CCommonCodeGenerator(const SCommonCodeConfiguration& codeConfig);
  ~CCommonCodeGenerator() override;

  QString Generate(const SBackgroundSnippetData& data, tspProject spCurrentProject) const override;
  QString Generate(const SDeviceSnippetData& data, tspProject spCurrentProject) const override;
  QString Generate(const SIconSnippetData& data, tspProject spCurrentProject) const override;
  QString Generate(const SMetronomeSnippetCode& data, tspProject spCurrentProject) const override;
  QString Generate(const SNotificationSnippetCode& data, tspProject spCurrentProject) const override;
  QString Generate(const SResourceSnippetData& data, tspProject spCurrentProject) const override;
  QString Generate(const STextSnippetCode& data, tspProject spCurrentProject) const override;
  QString Generate(const SThreadSnippetOverlay& data, tspProject spCurrentProject) const override;
  QString Generate(const STimerSnippetData& data, tspProject spCurrentProject) const override;

protected:
  QString Array(const QString& sContent) const;
  QString Assignment(const QString& sLhs, const QString& sRhs) const;
  QString Call(const QString& sContent) const;
  QString Comment(const QString& sContent) const;
  QString Invoke(const QString& sObj, const QString& sMember) const;
  QString Member(const QString& sObj, const QString& sMember) const;
  QString Local(const QString& sVariable) const;
  QString Statement(const QString& sContent, const QString& sBeforeNewLine = QString()) const;
  QString String(const QString& sContent) const;

  SCommonCodeConfiguration m_codeConfig;
};

#endif // CCOMMONCODEGENERATOR_H
