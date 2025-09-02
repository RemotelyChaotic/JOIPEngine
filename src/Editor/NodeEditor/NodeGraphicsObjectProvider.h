#ifndef CNODEGRAPHICSOBJECTPROVIDER_H
#define CNODEGRAPHICSOBJECTPROVIDER_H

#include <nodes/internal/NodeGraphicsObject.hpp>

#include <functional>
#include <memory>
#include <type_traits>

class CNodeGraphicsObjectProvider
{
protected:
  using tFnCreate = std::function<std::unique_ptr<QtNodes::NodeGraphicsObject>(
      QtNodes::FlowScene&,QtNodes::Node&)>;
public:

  CNodeGraphicsObjectProvider(const tFnCreate& fn);
  virtual ~CNodeGraphicsObjectProvider();

  std::unique_ptr<QtNodes::NodeGraphicsObject> createObject(QtNodes::FlowScene& scene,
                                                            QtNodes::Node& node) const;

private:
  tFnCreate m_fn;
};


template<typename T,
         typename std::enable_if_t<std::is_same_v<QtNodes::NodeGraphicsObject, T> ||
                                   std::is_base_of_v<QtNodes::NodeGraphicsObject, T>, int> = 0>
class CTypedNodeGraphicsObjectProvider : public CNodeGraphicsObjectProvider
{
public:
  CTypedNodeGraphicsObjectProvider() :
      CNodeGraphicsObjectProvider([](QtNodes::FlowScene& scene, QtNodes::Node& node)
                                    -> std::unique_ptr<QtNodes::NodeGraphicsObject>{
        return std::make_unique<T>(scene, node);
      })
  {}
  ~CTypedNodeGraphicsObjectProvider() override
  {
  }
};

class CDefaultGraphicsObjectProvider : public CTypedNodeGraphicsObjectProvider<QtNodes::NodeGraphicsObject>
{
public:
  CDefaultGraphicsObjectProvider();
  ~CDefaultGraphicsObjectProvider() override;
};

#endif // CNODEGRAPHICSOBJECTPROVIDER_H
