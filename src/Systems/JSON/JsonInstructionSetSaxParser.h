#ifndef JSONINSTRUCTIONSETSAXPARSER_H
#define JSONINSTRUCTIONSETSAXPARSER_H

#include <nlohmann/json.hpp>

#include <QString>

#include <map>
#include <memory>
#include <stack>

class CJsonInstructionSetRunnerPrivate;
class IJsonInstructionBase;
class CJsonInstructionNode;

class CJSonSaxParser : public nlohmann::json_sax<nlohmann::json>
{
public:
  CJSonSaxParser(CJsonInstructionSetRunnerPrivate* pParent,
                 const std::map<QString, std::shared_ptr<IJsonInstructionBase>>& instructionMap) :
    m_pParent(pParent),
    m_instructionMap(instructionMap)
  {}
  ~CJSonSaxParser() override {}

  // called when null is parsed
  bool null() override;
  // called when a boolean is parsed; value is passed
  bool boolean(bool val) override;

  // called when a signed or unsigned integer number is parsed; value is passed
  bool number_integer(number_integer_t val) override;
  bool number_unsigned(number_unsigned_t val) override;

  // called when a floating-point number is parsed; value and original string is passed
  bool number_float(number_float_t val, const string_t& s) override;
  // called when a string is parsed; value is passed and can be safely moved away
  bool string(string_t& val) override;
  // called when a binary value is parsed; value is passed and can be safely moved away
  bool binary(binary_t& val) override;

  // called when an object or array begins or ends, resp. The number of elements is passed (or -1 if not known)
  bool start_object(std::size_t elements) override;
  bool end_object() override;
  bool start_array(std::size_t elements) override;
  bool end_array() override;
  // called when an object key is parsed; value is passed and can be safely moved away
  bool key(string_t& val) override;
  // called when a parse error occurs; byte position, the last token, and an exception is passed
  bool parse_error(std::size_t position, const std::string& last_token,
                   const nlohmann::detail::exception& ex) override;

  void CreateAndAddIgnoreNode();

private:
  CJsonInstructionSetRunnerPrivate*                        m_pParent = nullptr;
  std::map<QString, std::shared_ptr<IJsonInstructionBase>> m_instructionMap;
  std::stack<std::shared_ptr<CJsonInstructionNode>>        m_parseStack;
  std::string                                              m_sCurrentKey;
  bool                                                     m_bParsingCommands = false;
};

#endif // JSONINSTRUCTIONSETSAXPARSER_H
