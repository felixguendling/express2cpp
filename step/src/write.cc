#include "step/write.h"

#include "utl/enumerate.h"

#include "step/model.h"
#include "step/root_entity.h"

namespace step {

void write(std::ostream& out, model const& m) {
  write_context ctx;
  for (auto const& [i, e] : utl::enumerate(m.entity_mem_)) {
    ctx.ptr_to_id_.emplace(e.get(), i);
  }
  for (auto const& [i, e] : utl::enumerate(m.entity_mem_)) {
    out << "#" << i << " = ";
    e->write(ctx, out, true);
    out << "\n";
  }
}

}  // namespace step