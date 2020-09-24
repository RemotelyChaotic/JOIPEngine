#ifndef CJSONINSTRUCTIONSETPARSER_H
#define CJSONINSTRUCTIONSETPARSER_H

#include "JsonInstructionBase.h"
#include <QJsonDocument>
#include <QObject>
#include <memory>

class CJsonInstructionSetRunner;
class CJsonInstructionSetParserPrivate;

class CJsonInstructionSetParser : public QObject
{
  Q_OBJECT
public:
  explicit CJsonInstructionSetParser(QObject* pParent = nullptr);
  ~CJsonInstructionSetParser() override;

  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const QByteArray& json);
  void RegisterInstructionSchema(const QString& sId, const JsonInstructionBase& instructionDefinition);
  void SetJsonBaseSchema(const QByteArray& schema);

private:
  std::unique_ptr<CJsonInstructionSetParserPrivate> m_spPtr;
};

#endif // CJSONINSTRUCTIONSETPARSER_H
