#ifndef ONEFLOW_CORE_GRAPH_LOGICAL_NODE_H_
#define ONEFLOW_CORE_GRAPH_LOGICAL_NODE_H_

#include "oneflow/core/graph/boxing_task_node.h"
#include "oneflow/core/graph/compute_task_node.h"
#include "oneflow/core/operator/operator.h"

namespace oneflow {

class LogicalEdge;

class LogicalNode : public Node<LogicalNode, LogicalEdge> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(LogicalNode);
  virtual ~LogicalNode() = default;

  // op_vec_
  std::shared_ptr<Operator> SoleOp() const;
  const std::vector<std::shared_ptr<Operator>>& op_vec() const { return op_vec_; }
  std::vector<std::shared_ptr<Operator>>& mut_op_vec() { return op_vec_; }

  // parallel_desc_
  std::shared_ptr<const ParallelDesc> parallel_desc() const { return parallel_desc_; }
  std::shared_ptr<const ParallelDesc>& mut_parallel_desc() { return parallel_desc_; }

  // shared_model_nodes_
  std::shared_ptr<const std::vector<LogicalNode*>> shared_model_nodes() const {
    return shared_model_nodes_;
  }
  std::shared_ptr<const std::vector<LogicalNode*>>& mut_shared_model_nodes() {
    return shared_model_nodes_;
  }

  // Lbis
  std::vector<LogicalBlobId> GetLbisTo(const LogicalNode* dst) const;
  void SetDataLbisTo(const LogicalNode* dst, const std::vector<LogicalBlobId>&);
  const HashSet<LogicalBlobId>& lbi_boxing() const { return lbi_boxing_; }
  const HashSet<LogicalBlobId>& lbi_121() const { return lbi_121_; }

  // util
  virtual std::string TypeName() const = 0;
  std::string VisualStr() const;
  bool HasOpWithModelOrModelTmpBlob() const;
  bool HasOpWithModelBlob() const;
  bool HasOpWithForwardModelBlob() const;
  void GenSortedCompTaskNodes(std::function<int64_t(const TaskNode*)> AllocateCpuThrdId,
                              std::function<void(CompTaskNode*)>) const;

  // model split
  LogicalNode* main_model_parallel() const { return main_model_parallel_; }
  void set_main_model_parallel(LogicalNode* val) { main_model_parallel_ = val; }
  int32_t GetModelSplitAxis() const;
  int32_t GetMaxModelSplitNum() const;

 protected:
  LogicalNode() : main_model_parallel_(nullptr) {}
  virtual CompTaskNode* NewCompTaskNode() const = 0;
  virtual void FixCompTaskNode(CompTaskNode*) const {}

 private:
  bool HasOpWithCondition(std::function<bool(const Operator*)>) const;

  std::vector<std::shared_ptr<Operator>> op_vec_;
  std::shared_ptr<const ParallelDesc> parallel_desc_;
  std::shared_ptr<const std::vector<LogicalNode*>> shared_model_nodes_;
  LogicalNode* main_model_parallel_;

  HashMap<const LogicalNode*, std::vector<LogicalBlobId>> dst2data_lbis_;
  HashSet<LogicalBlobId> lbi_boxing_;
  HashSet<LogicalBlobId> lbi_121_;
};

#define BLD_SUB_TSK_GPH_MTHD_ARGS()                                                       \
  (const LogicalNode* src_logical, const LogicalNode* dst_logical,                        \
   const std::vector<CompTaskNode*>& sorted_src_comp_tasks,                               \
   const std::vector<CompTaskNode*>& sorted_dst_comp_tasks,                               \
   HashMap<const LogicalNode*, std::vector<TaskNode*>>* logical2sorted_in_box,            \
   HashMap<const LogicalNode*, std::vector<TaskNode*>>* logical2sorted_out_box,           \
   std::function<TaskNode**(CompTaskNode * src, int64_t machine_id, int32_t mem_zone_id)> \
       Mut121BufTask,                                                                     \
   std::function<int64_t(const TaskNode*)> AllocateCpuThrdId)

class TaskGraph;
using BldSubTskGphMthd = void(TaskGraph::*) BLD_SUB_TSK_GPH_MTHD_ARGS();

class LogicalEdge final : public Edge<LogicalNode, LogicalEdge> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(LogicalEdge);
  LogicalEdge() = default;
  ~LogicalEdge() = default;

  const LogicalBlobId& SoleLbi() const {
    CHECK_EQ(lbis_.size(), 1);
    return lbis_.front();
  }

  const std::vector<LogicalBlobId>& lbis() const { return lbis_; }
  std::vector<LogicalBlobId>& mut_lbis() { return lbis_; }

 private:
  std::vector<LogicalBlobId> lbis_;
};

BldSubTskGphMthd GetMthdForBldSubTskGph(const LogicalNode* src, const LogicalNode* dst);

using BldBoxingOpConfMthd = void (BoxingTaskNode::*)(
    const LogicalBlobId& lbi, const std::vector<BoxingTaskNode::EdgeInfo>& sorted_in_edges,
    const LogicalNode* in_logical, const std::vector<BoxingTaskNode::EdgeInfo>& sorted_out_edges,
    const LogicalNode* out_logical, BoxingOpConf*);
BldBoxingOpConfMthd GetMthdForBldBoxingOpConf(const LogicalNode* src, const LogicalNode* dst);

#define OVERRIDE_PURE_VIRTUAL_METHOD()   \
  std::string TypeName() const override; \
  CompTaskNode* NewCompTaskNode() const override;

#define LOGICAL_NODE_BOILERPLATE(class_name) \
  OF_DISALLOW_COPY_AND_MOVE(class_name);     \
  class_name() = default;                    \
  ~class_name() = default;                   \
  OVERRIDE_PURE_VIRTUAL_METHOD();

class BackwardLogicalNode;

class ForwardLogicalNode : public LogicalNode {
 public:
  OF_DISALLOW_COPY_AND_MOVE(ForwardLogicalNode);
  ForwardLogicalNode() : bw_node_(nullptr) {}
  virtual ~ForwardLogicalNode() = default;

  BackwardLogicalNode* bw_node() const { return bw_node_; }

  BackwardLogicalNode* NewBackwardNode();

 protected:
  virtual BackwardLogicalNode* NewCorrectBackwardNode() = 0;

 private:
  BackwardLogicalNode* bw_node_;
};

class NormalForwardLogicalNode final : public ForwardLogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(NormalForwardLogicalNode);

  BackwardLogicalNode* NewCorrectBackwardNode() override;

 private:
};

class BackwardLogicalNode : public LogicalNode {
 public:
  OF_DISALLOW_COPY_AND_MOVE(BackwardLogicalNode);
  BackwardLogicalNode() : fw_node_(nullptr) {}
  virtual ~BackwardLogicalNode() = default;

  ForwardLogicalNode* fw_node() const { return fw_node_; }

 private:
  friend class ForwardLogicalNode;

  ForwardLogicalNode* fw_node_;
};

class NormalBackwardLogicalNode final : public BackwardLogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(NormalBackwardLogicalNode);
};

class RecordLoadLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(RecordLoadLogicalNode);
};

class DecodeLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(DecodeLogicalNode);
};

class LossLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(LossLogicalNode);
};

class PrintLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(PrintLogicalNode);
};

class LossAccLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(LossAccLogicalNode);
};

class LossPrintLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(LossPrintLogicalNode);
};

class NormalMdUpdtLogicalNode final : public LogicalNode {
 public:
  OF_DISALLOW_COPY_AND_MOVE(NormalMdUpdtLogicalNode);
  NormalMdUpdtLogicalNode() : random_seed_(NewRandomSeed()) {}
  ~NormalMdUpdtLogicalNode() = default;

  OVERRIDE_PURE_VIRTUAL_METHOD();

 private:
  void FixCompTaskNode(CompTaskNode*) const override;

  uint32_t random_seed_;
};

class MdSaveLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(MdSaveLogicalNode);
};

class MdDiffAccLogicalNode final : public LogicalNode {
 public:
  LOGICAL_NODE_BOILERPLATE(MdDiffAccLogicalNode);
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_GRAPH_LOGICAL_NODE_H_