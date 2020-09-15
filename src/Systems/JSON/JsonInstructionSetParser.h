#ifndef CJSONINSTRUCTIONSETPARSER_H
#define CJSONINSTRUCTIONSETPARSER_H

#include <QIODevice>
#include <QObject>
#include <memory>

class CJsonInstructionSetRunner;

class CJsonInstructionSetParser : public QObject
{
  Q_OBJECT
public:
  explicit CJsonInstructionSetParser(QObject* pParent = nullptr);
  ~CJsonInstructionSetParser() override;

  void SetJsonSchema(const QByteArray& json);
  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const QByteArray& json);
};

#endif // CJSONINSTRUCTIONSETPARSER_H
