#ifndef _HLIR_OPTIMIZER_H
#define _HLIR_OPTIMIZER_H

#include "hlir.h"
#include "optimizer_config.h"

namespace hlir {

enum class PassScope {
  Method,
  Class,
  Module,
};

class Pass {
public:
  std::string name;
  PassScope pass_scope;

  Pass(std::string n, PassScope ps);

  virtual void run(hlir::Universe &, const OptimizerConfig &) const;
  virtual void run_class(hlir::Class &, const OptimizerConfig &) const;
  virtual void run_method(hlir::InstructionList &,
                          const OptimizerConfig &) const;
};

class PassManager {
private:
  std::vector<std::unique_ptr<Pass>> pass_pipeline;
  hlir::Universe &universe;
  const OptimizerConfig &config;

  int current_pass;

public:
  PassManager(hlir::Universe &, const OptimizerConfig &);

  bool is_done();

  const Pass &run_pass();
};

} // namespace hlir

#endif // !_HLIR_OPTIMIZER_H
