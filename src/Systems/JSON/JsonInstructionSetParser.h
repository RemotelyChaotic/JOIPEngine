#ifndef CJSONINSTRUCTIONSETPARSER_H
#define CJSONINSTRUCTIONSETPARSER_H

#include "JsonInstructionBase.h"
#include <QJsonDocument>
#include <QObject>
#include <memory>
#include <type_traits>

class CJsonInstructionSetRunner;
class CJsonInstructionSetParserPrivate;

class CJsonInstructionSetParser : public QObject
{
  Q_OBJECT
public:
  explicit CJsonInstructionSetParser(QObject* pParent = nullptr);
  ~CJsonInstructionSetParser() override;

  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const QByteArray& json);
  template<typename T,
           typename = typename std::enable_if<std::is_base_of<IJsonInstructionBase, T>::value, void>::type>
  void RegisterInstruction(const QString& sId)
  {
    RegisterInstruction(sId, std::make_shared<T>());
  }
  void RegisterInstruction(const QString& sId,
                           const std::shared_ptr<IJsonInstructionBase>& spInstructionDefinition);
  void RegisterInstructionSetPath(const QString& sId, const QString& sInstructionSetPath);
  void SetJsonBaseSchema(const QByteArray& schema);

private:
  std::unique_ptr<CJsonInstructionSetParserPrivate> m_spPtr;
};

#endif // CJSONINSTRUCTIONSETPARSER_H
