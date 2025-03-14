// Copyright 2021 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef XLS_CODEGEN_CONVERSION_UTILS_H_
#define XLS_CODEGEN_CONVERSION_UTILS_H_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "xls/codegen/codegen_options.h"
#include "xls/codegen/codegen_pass.h"
#include "xls/ir/block.h"
#include "xls/ir/node.h"
#include "xls/ir/xls_ir_interface.pb.h"

namespace xls::verilog {

// If specified by user, returns the PackageInterFaceProto::Function for the
// function with the given name.
std::optional<PackageInterfaceProto::Function> FindFunctionInterface(
    const std::optional<PackageInterfaceProto>& src,
    std::string_view func_name);

// If specified by user, returns the PackageInterFaceProto::Channel for the
// channel with the given name.
std::optional<PackageInterfaceProto::Channel> FindChannelInterface(
    const std::optional<PackageInterfaceProto>& src,
    std::string_view chan_name);

// Returns pipeline-stage prefixed signal name with the given string as the
// root. For example, for root `foo` the name might be: p3_foo. Nodes in the
// block generated by FunctionBaseToPipelinedBlock and
// PackageToPipelinedBlocks are named using this function.
std::string PipelineSignalName(std::string_view root, int64_t stage);

// For each output streaming channel add a corresponding ready port (input
// port). Combinationally combine those ready signals with their predicates to
// generate an  all_active_outputs_ready signal.
//
// Upon success returns a Node* to the all_active_inputs_valid signal.
absl::StatusOr<std::vector<Node*>> MakeInputReadyPortsForOutputChannels(
    std::vector<std::vector<StreamingOutput>>& streaming_outputs,
    int64_t stage_count, std::string_view ready_suffix, Block* block);

absl::StatusOr<std::vector<Node*>> MakeInputValidPortsForInputChannels(
    std::vector<std::vector<StreamingInput>>& streaming_inputs,
    int64_t stage_count, std::string_view valid_suffix, Block* block);

// Given a node returns a node that is OR'd with the reset signal.
// if said reset signal exists.  That node can be thought of as
//     1 - If being reset or if the src_node is 1
//     0 - otherwise.
//
//   - If no reset exists, the node is returned and the graph unchanged.
//   - Active low reset signals are inverted so that the resulting signal
//      OR(src_node, NOT(reset))
//
// This is used to drive load_enable signals of pipeline valid registers.
absl::StatusOr<Node*> MakeOrWithResetNode(Node* src_node,
                                          std::string_view result_name,
                                          Block* block);

// Returns or makes a node that is 1 when the block is under reset, if said
// reset signal exists.
//
//   - If no reset exists, std::nullopt is returned
//   - Active low reset signals are inverted.
//
// See also MakeOrWithResetNode() or ResetNotAsserted().
absl::StatusOr<std::optional<Node*>> ResetAsserted(Block* block);

// Returns or makes a node that is 1 when the block is not under reset, if said
// reset signal exists.
//
//   - If no reset exists, std::nullopt is returned
//   - Active high reset signals are inverted.
//
// See also ResetAsserted().
absl::StatusOr<std::optional<Node*>> ResetNotAsserted(Block* block);

// If options specify it, adds and returns an input for a reset signal.
absl::Status MaybeAddResetPort(Block* block, const CodegenOptions& options);

// Send/receive nodes are not cloned from the proc into the block, but the
// network of tokens connecting these send/receive nodes *is* cloned. This
// function removes the token operations.
absl::Status RemoveDeadTokenNodes(CodegenPassUnit* unit);

// Make valid ports (output) for the output channel.
//
// A valid signal is asserted iff all active
// inputs valid signals are asserted and the predicate of the data channel (if
// any) is asserted.
absl::Status MakeOutputValidPortsForOutputChannels(
    absl::Span<Node* const> all_active_inputs_valid,
    absl::Span<Node* const> pipelined_valids,
    absl::Span<Node* const> next_stage_open,
    std::vector<std::vector<StreamingOutput>>& streaming_outputs,
    std::string_view valid_suffix, Block* block);

// Make ready ports (output) for each input channel.
//
// A ready signal is asserted iff all active
// output ready signals are asserted and the predicate of the data channel (if
// any) is asserted.
absl::Status MakeOutputReadyPortsForInputChannels(
    absl::Span<Node* const> all_active_outputs_ready,
    std::vector<std::vector<StreamingInput>>& streaming_inputs,
    std::string_view ready_suffix, Block* block);
}  // namespace xls::verilog

#endif  // XLS_CODEGEN_CONVERSION_UTILS_H_
