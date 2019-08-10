#ifndef SCENETRANSTITIONDATA_H
#define SCENETRANSTITIONDATA_H

#include <nodes/NodeDataModel>

using QtNodes::NodeDataType;
using QtNodes::NodeData;

class CSceneTranstitionData : public NodeData
{
public:
  CSceneTranstitionData();
  ~CSceneTranstitionData() override;

  NodeDataType type() const override;
};

#endif // SCENETRANSTITIONDATA_H
