/* Copyright 2022 NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "legate.h"
#include "sparse_c.h"
#include "sparse/mapper/mapper.h"

using namespace legate;
using namespace legate::mapping;

namespace sparse {

LegateSparseMapper::LegateSparseMapper(Legion::Runtime* rt,
                                       Legion::Machine m,
                                       const LibraryContext& ctx)
  : BaseMapper(rt, m, ctx)
{
}

TaskTarget LegateSparseMapper::task_target(const Task& task, const std::vector<TaskTarget>& options)
{
  return *options.begin();
}

std::vector<StoreMapping> LegateSparseMapper::store_mappings(
  const Task& task, const std::vector<StoreTarget>& options)
{
  // Just do the default thing for now.
  auto& inputs = task.inputs();
  std::vector<StoreMapping> mappings(inputs.size());
  for (size_t i = 0; i < task.inputs().size(); i++) {
    mappings[i] = StoreMapping::default_mapping(inputs[i], options.front());
  }
  return mappings;
}

Scalar LegateSparseMapper::tunable_value(legate::TunableID tunable_id)
{
  switch (tunable_id) {
    case LEGATE_SPARSE_TUNABLE_NUM_PROCS: {
      int32_t num_procs = 0;
      if (!local_gpus.empty())
        num_procs = local_gpus.size() * total_nodes;
      else if (!local_omps.empty())
        num_procs = local_omps.size() * total_nodes;
      else
        num_procs = local_cpus.size() * total_nodes;
      return Scalar(num_procs);
    }
    case LEGATE_SPARSE_TUNABLE_HAS_NUMAMEM: {
      Legion::Machine::MemoryQuery query(machine);
      query.local_address_space();
      query.only_kind(Legion::Memory::SOCKET_MEM);
      int32_t has_numamem = query.count() > 0;
      return Scalar(has_numamem);
    }
    case LEGATE_SPARSE_TUNABLE_NUM_GPUS: {
      int32_t num_gpus = local_gpus.size() * total_nodes;
      return Scalar(num_gpus);
    }
    default: break;
  }
  LEGATE_ABORT;
}

}  // namespace sparse