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

Pass::Pass(std::string n, PassScope ps) : name(n), pass_scope(ps) {}

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

// UselessAccMov
//
// Peephole around instructions, removing unnecessary Mov operations into acc.
//
// Catches moves into acc that are only used once and immediately rewritten, or
// never used at all.

class UselessAccMov : public Pass {
public:
  UselessAccMov() : Pass("useless_acc_mov", PassScope::Method) {}
  void run_method(InstructionList &, const OptimizerConfig &) const override;
};

void UselessAccMov::run_method(hlir::InstructionList &instructions,
                               const OptimizerConfig &config) const {
  auto instruction_it = instructions.begin();

  while (instruction_it != instructions.end()) {
    hlir::Instruction *instruction = instruction_it->get();

    // Start by matching sequences of MOV + mod acc. We'll skip everything else
    if (instruction->op != Op::MOV ||
        instruction->get_dest().kind != ValueKind::ACC) {
      instruction_it++;
      continue;
    }

    hlir::Instruction *lookahead = std::next(instruction_it, 1)->get();
    int lookahead_args = lookahead->num_args();

    if (!lookahead->has_dest() ||
        lookahead->get_dest().kind != ValueKind::ACC) {
      instruction_it = std::next(instruction_it, 2);
      continue;
    }

    // acc is reset immediately without using. Simply delete
    if (lookahead_args == 0) {
      instruction_it = instructions.erase(instruction_it);
      continue;
    }

    Value stored = instruction->get_arg1();

    if (lookahead_args == 1 && lookahead->get_arg1().kind == ValueKind::ACC) {
      Value &useless_acc_arg = lookahead->get_arg1();

      instruction_it = instructions.erase(instruction_it);
      useless_acc_arg = stored; // replace useless_acc_arg

      continue;
    }

    if (lookahead_args == 2) {
      Value &arg1 = lookahead->get_arg1();
      Value &arg2 = lookahead->get_arg2();
      if (arg1.kind == ValueKind::ACC)
        arg1 = stored;
      if (arg2.kind == ValueKind::ACC)
        arg2 = stored;

      instruction_it = instructions.erase(instruction_it);
    }
  }
};

/**********************
 *                    *
 *    PassManager     *
 *                    *
 *********************/

PassManager::PassManager(hlir::Universe &u, const OptimizerConfig &oc)
    : universe(u), config(oc), current_pass(0) {
  // TODO(IT) consider OptimizerConfig for setting passes
  pass_pipeline.push_back(std::make_unique<UselessAccMov>());
}

bool PassManager::is_done() { return current_pass >= pass_pipeline.size(); }

const hlir::Pass &PassManager::run_pass() {
  hlir::Pass *pass_to_run = pass_pipeline[current_pass].get();
  pass_to_run->run(universe, config);

  current_pass++;

  return *pass_to_run;
}

}; // namespace hlir
