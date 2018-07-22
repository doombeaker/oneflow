#include "oneflow/core/graph/regst_lifetime_graph.h"

namespace oneflow {

RegstLifetimeGraph::RegstLifetimeGraph(
    const std::list<const RegstDescProto*>& regst_descs,
    const std::function<void(const RegstDescProto*, HashSet<int64_t>*)>& ComputeLifetimeActorIds) {
  std::list<RegstLifetimeNode*> nodes;
  InitNodes(regst_descs, ComputeLifetimeActorIds, &nodes);
  InitEdges(nodes);
}

void RegstLifetimeGraph::InitNodes(
    const std::list<const RegstDescProto*>& regst_descs,
    const std::function<void(const RegstDescProto*, HashSet<int64_t>*)>& ComputeLifetimeActorIds,
    std::list<RegstLifetimeNode*>* nodes) {
  for (const RegstDescProto* regst_desc : regst_descs) {
    auto lifetime_actor_ids = std::make_unique<HashSet<int64_t>>();
    ComputeLifetimeActorIds(regst_desc, lifetime_actor_ids.get());
    auto* node = new RegstLifetimeNode(regst_desc, std::move(lifetime_actor_ids));
    AddAllocatedNode(node);
    nodes->push_back(node);
  }
}

void RegstLifetimeGraph::InitEdges(const std::list<RegstLifetimeNode*>& nodes) {
  HashMap<int64_t, HashSet<RegstLifetimeNode*>> task_id2intersected_nodes;
  for (RegstLifetimeNode* node : nodes) {
    for (int64_t task_id : node->lifetime_actor_ids()) {
      task_id2intersected_nodes[task_id].insert(node);
    }
  }
  HashMap<RegstLifetimeNode*, HashSet<RegstLifetimeNode*>> src_node2dst_nodes;
  for (const auto& pair : task_id2intersected_nodes) {
    for (RegstLifetimeNode* src_node : pair.second) {
      for (RegstLifetimeNode* dst_node : pair.second) {
        if (src_node->regst_desc_id() < dst_node->regst_desc_id()) {
          src_node2dst_nodes[src_node].emplace(dst_node);
        }
      }
    }
  }
  for (const auto& pair : src_node2dst_nodes) {
    for (RegstLifetimeNode* dst_node : pair.second) { Connect(pair.first, NewEdge(), dst_node); }
  }
}

void RegstLifetimeGraph::ForEachSameColoredRegstDescs(
    const std::function<void(const std::list<const RegstDescProto*>&)>& Handler) const {
  HashMap<const RegstLifetimeNode*, std::set<int32_t>> node2excluded_color_ids;
  HashMap<const RegstLifetimeNode*, int32_t> node2color_id;
  auto ForEachIntersected = &RegstLifetimeNode::ForEachNodeOnInOutEdge;
  ForEachNode([&](const RegstLifetimeNode* start) {
    if (node2color_id.find(start) != node2color_id.end()) { return; }
    BfsForEachNode({start}, ForEachIntersected, [&](const RegstLifetimeNode* node) {
      if (node2color_id.find(node) != node2color_id.end()) { return; }
      int32_t color_id = 0;
      const auto& excluded_color_ids = node2excluded_color_ids[node];
      for (; excluded_color_ids.find(color_id) != excluded_color_ids.end(); ++color_id) {}
      node2color_id[node] = color_id;
      (node->*ForEachIntersected)([&](const RegstLifetimeNode* intersected) {
        if (node2color_id.find(intersected) != node2color_id.end()) { return; }
        node2excluded_color_ids[intersected].insert(color_id);
      });
    });
  });
  HashMap<int32_t, std::list<const RegstDescProto*>> color_id2regst_descs;
  for (const auto& pair : node2color_id) {
    color_id2regst_descs[pair.second].push_back(&pair.first->regst_desc());
  }
  for (const auto& pair : color_id2regst_descs) { Handler(pair.second); }
}

}  // namespace oneflow