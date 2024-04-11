#include "hlir_optimizer.h"
#include "error.h"
#include "optimizer_config.h"
#include <format>

namespace hlir {

/**********************
 *                    *
 *        Pass        *
 *                    *
 *********************/

void Pass::run(hlir::Universe &universe, const OptimizerConfig &config) const {
  for (auto &[_, cls] : universe.classes) {
    run_class(cls, config);
  }
}
void Pass::run_class(hlir::Class &cls, const OptimizerConfig &config) const {
  run_method(cls.initializer, config);
  for (auto &[_, method] : cls.methods) {
    run_method(method.instructions, config);
  }
}
void Pass::run_method(hlir::InstructionList &, const OptimizerConfig &) const {
  fatal(std::format("INTERNAL: trying to run undefined run_method in Pass {}",
                    name));
}

/**********************
 *                    *
 *  Optimizer Passes  *
 *                    *
 *********************/

//
// IrrelevantAcc
//
// Peephole around instructions, removing unnecessary Mov operations into acc.
//
// Catches moves into acc that are only used once and immediately rewritten.

class IrrelevantAcc : public Pass {
public:
  void run_method(InstructionList &, const OptimizerConfig &) const override;
};

void IrrelevantAcc::run_method(hlir::InstructionList &instructions,
                               const OptimizerConfig &config) const {
  auto instruction_it = instructions.begin();

  while (instruction_it != instructions.end()) {

    // Start by matching sequences of MOV-X-MOV. We'll skip everything else

    if (instruction_it->get()->op != Op::MOV) {
      instruction_it++;
      continue;
    }

    if (std::next(instruction_it, 2)->get()->op != Op::MOV) {
      instruction_it = std::next(instruction_it, 3);
      continue;
    }

    // We have matched the MOVs, dynamic cast and do your magic
  }
};

}; // namespace hlir
