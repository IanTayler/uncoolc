#include "semantic.h"
#include "error.h"
#include <format>
#include <stdexcept>
/***********************
 *                     *
 *        Scope        *
 *                     *
 **********************/

Scopes::Scopes() {}

void Scopes::enter() { scopes.push_front(std::unordered_map<int, Symbol>()); }
void Scopes::exit() { scopes.pop_front(); }

void Scopes::assign(Symbol name, Symbol type) {
  scopes.front()[name.id] = type;
}

Symbol Scopes::get(Symbol name) const {
  for (auto &scope : scopes) {
    if (scope.find(name.id) != scope.end()) {
      return scope.at(name.id);
    }
  }
  return Symbol{};
}

Symbol Scopes::lookup(Symbol name) const {
  try {
    return scopes.front().at(name.id);
  } catch (const std::out_of_range &) {
    return Symbol{};
  }
}

/***********************
 *                     *
 *      ClassInfo      *
 *                     *
 **********************/

ClassInfo::ClassInfo(ClassNode *cn, int d)
    : class_node(cn), depth_(d),
      methods(std::unordered_map<int, MethodNode *>()),
      attributes(std::unordered_map<int, AttributeNode *>()) {
  for (const auto &method_ptr : cn->methods) {
    Symbol name = method_ptr->name;
    MethodNode *raw_method_ptr = method_ptr.get();
    methods[name.id] = raw_method_ptr;
  }

  for (const auto &attr_ptr : cn->attributes) {
    Symbol object_id = attr_ptr->object_id;
    AttributeNode *raw_attr_ptr = attr_ptr.get();
    attributes[object_id.id] = raw_attr_ptr;
  }
}

unsigned int ClassInfo::start_line() const {
  return class_node->start_token.line();
}

unsigned int ClassInfo::start_column() const {
  return class_node->start_token.column();
}

int ClassInfo::depth() const { return depth_; }

Symbol ClassInfo::name() const { return class_node->name; }

Symbol ClassInfo::superclass() const { return class_node->superclass; }

MethodNode *ClassInfo::method(Symbol name) { return methods[name.id]; }

AttributeNode *ClassInfo::attribute(Symbol name) { return attributes[name.id]; }

/***********************
 *                     *
 *      ClassTree      *
 *                     *
 **********************/
ClassTree::ClassTree(ModuleNode *module, SymbolTable &symbs) : symbols(symbs) {
  if (module == nullptr) {
    fatal("INTERNAL: Null module passed to create ClassTree", Token{});
  }

  add_default_classes();

  std::unordered_map<int, ClassNode *> class_node_map =
      get_class_node_map(module);

  check_class_hierarchy(class_node_map, module);

  std::vector<Symbol> classes_by_depth = get_classes_by_depth(module);

  for (const auto &cls_name : classes_by_depth) {
    ClassNode *class_node = class_node_map[cls_name.id];

    Token class_token = class_node->start_token;
    std::optional<ClassInfo> stored_class = get(class_node->name);

    if (stored_class.has_value()) {
      ClassInfo cls = stored_class.value();

      if (cls.start_line() == class_token.line() &&
          cls.start_column() == class_token.column()) {
        continue;

      } else {
        fatal(std::format("Redefinition of class {}",
                          symbols.get_string(class_node->name)),
              class_token);
      }
    }

    int depth;

    std::optional<ClassInfo> stored_superclass = get(class_node->superclass);

    if (stored_superclass.has_value()) {
      depth = stored_superclass.value().depth() + 1;
    } else {
      fatal(std::format("INTERNAL: Undefined superclass {} after checks",
                        symbols.get_string(class_node->superclass)),
            class_node->start_token);
    }

    add_class(class_node, depth);
  }
}

std::unordered_map<int, ClassNode *>
ClassTree::get_class_node_map(ModuleNode *module) const {
  std::unordered_map<int, ClassNode *> class_node_map;

  for (const auto &class_node : module->classes) {
    class_node_map[class_node->name.id] = class_node.get();
  }

  return class_node_map;
}

std::vector<Symbol> ClassTree::get_classes_by_depth(ModuleNode *module) const {
  std::unordered_map<int, std::vector<Symbol>> dependants;
  for (const auto &class_node : module->classes) {
    dependants[class_node->superclass.id].push_back(class_node->name);
  }

  std::vector<Symbol> classes_by_depth;

  // First add all the default classes. We will iterate through them first.
  for (const auto &cls : classes) {
    classes_by_depth.push_back(cls.name());
  }

  // Now add all dependants in order: superclass precedes subclass
  for (int i = 0; i < classes_by_depth.size(); i++) {
    Symbol class_name = classes_by_depth[i];
    for (const auto &subclass : dependants[class_name.id]) {
      classes_by_depth.push_back(subclass);
    }
  }

  return classes_by_depth;
}

void ClassTree::check_class_hierarchy(
    const std::unordered_map<int, ClassNode *> &class_node_map,
    ModuleNode *module) {
  for (const auto &class_node : module->classes) {
    Symbol superclass_name = class_node->superclass;
    if (class_node_map.find(superclass_name.id) == class_node_map.end()) {
      fatal(std::format("Undefined superclass {} for class {}",
                        symbols.get_string(class_node->superclass),
                        symbols.get_string(class_node->name)),
            class_node->start_token);
    }

    if (superclass_name == symbols.int_type ||
        superclass_name == symbols.bool_type ||
        superclass_name == symbols.string_type) {
      fatal(std::format("Cannot inherit from special class {}",
                        symbols.get_string(superclass_name)),
            class_node->start_token);
    }
  }
}

std::unique_ptr<MethodNode>
ClassTree::make_builtin_method(Symbol cls, Symbol name, Symbol return_type,
                               std::vector<Symbol> parameter_names,
                               std::vector<Symbol> parameter_types) {

  if (parameter_names.size() != parameter_types.size()) {
    fatal(
        std::format("INTERNAL: invalid builtin specification for {}.{}. "
                    "parameter names size does not match parameter_types size",
                    symbols.get_string(cls), symbols.get_string(name)),
        Token{});
  }

  std::vector<std::unique_ptr<ParameterNode>> parameters;

  for (int i = 0; i < parameter_names.size(); i++) {
    parameters.push_back(std::make_unique<ParameterNode>(
        parameter_names[i], parameter_types[i], Token{}));
  }

  return std::make_unique<MethodNode>(name, return_type, std::move(parameters),
                                      std::make_unique<BuiltinNode>(cls, name),
                                      Token{});
}

void ClassTree::add_default_classes() {
  // Object
  objectClassNode = std::make_unique<ClassNode>(
      symbols.object_type, symbols.tree_root_type, Token{});

  objectClassNode->methods.push_back(make_builtin_method(
      symbols.object_type, symbols.from("abort"), symbols.object_type, {}, {}));

  objectClassNode->methods.push_back(
      make_builtin_method(symbols.object_type, symbols.from("type_name"),
                          symbols.string_type, {}, {}));

  objectClassNode->methods.push_back(make_builtin_method(
      symbols.object_type, symbols.from("copy"), symbols.self_type, {}, {}));

  // IO
  ioClassNode = std::make_unique<ClassNode>(symbols.io_type,
                                            symbols.object_type, Token{});

  ioClassNode->methods.push_back(make_builtin_method(
      symbols.io_type, symbols.from("out_string"), symbols.self_type,
      {symbols.from("x")}, {symbols.string_type}));

  ioClassNode->methods.push_back(make_builtin_method(
      symbols.io_type, symbols.from("out_int"), symbols.self_type,
      {symbols.from("x")}, {symbols.int_type}));

  ioClassNode->methods.push_back(make_builtin_method(
      symbols.io_type, symbols.from("in_string"), symbols.string_type, {}, {}));

  ioClassNode->methods.push_back(make_builtin_method(
      symbols.io_type, symbols.from("in_int"), symbols.int_type, {}, {}));

  // String
  stringClassNode = std::make_unique<ClassNode>(symbols.string_type,
                                                symbols.object_type, Token{});

  stringClassNode->methods.push_back(make_builtin_method(
      symbols.string_type, symbols.from("length"), symbols.int_type, {}, {}));

  stringClassNode->methods.push_back(make_builtin_method(
      symbols.string_type, symbols.from("concat"), symbols.string_type,
      {symbols.from("s")}, {symbols.string_type}));

  stringClassNode->methods.push_back(make_builtin_method(
      symbols.string_type, symbols.from("substr"), symbols.string_type,
      {symbols.from("i"), symbols.from("l")},
      {symbols.int_type, symbols.int_type}));

  // Int
  intClassNode = std::make_unique<ClassNode>(symbols.int_type,
                                             symbols.object_type, Token{});

  // Bool
  boolClassNode = std::make_unique<ClassNode>(symbols.bool_type,
                                              symbols.object_type, Token{});

  // add all classes
  add_class(objectClassNode.get(), 0);
  add_class(ioClassNode.get(), 1);
  add_class(stringClassNode.get(), 1);
  add_class(intClassNode.get(), 1);
  add_class(boolClassNode.get(), 1);
}

void ClassTree::add_class(ClassNode *class_node, int depth) {
  int next_position = classes.size();

  classes.push_back(ClassInfo(class_node, depth));
  classes_by_name[class_node->name.id] = ClassIdx(next_position);
}

bool ClassTree::exists(Symbol name) const {
  if (classes_by_name.find(name.id) == classes_by_name.end())
    return false;
  return exists(classes_by_name.at(name.id));
}
bool ClassTree::exists(ClassIdx idx) const { return idx < classes.size(); }

std::optional<ClassInfo> ClassTree::get(Symbol name) const {
  if (exists(name)) {
    ClassIdx idx = classes_by_name.at(name.id);
    return get(idx);
  }
  return std::nullopt;
}

std::optional<ClassInfo> ClassTree::get(ClassIdx idx) const {
  if (exists(idx))
    return classes[idx];
  return std::nullopt;
}

std::optional<ClassInfo> ClassTree::common_ancestor(ClassIdx node_a,
                                                    ClassIdx node_b) const {
  if (!exists(node_a) || !exists(node_b))
    return std::nullopt;
  return common_ancestor(classes[node_a], classes[node_b]);
}

std::optional<ClassInfo> ClassTree::common_ancestor(Symbol name_a,
                                                    Symbol name_b) const {
  const auto &class_a = get(name_a);
  if (!class_a.has_value())
    return std::nullopt;
  const auto &class_b = get(name_b);
  if (!class_b.has_value())
    return std::nullopt;
  return common_ancestor(class_a.value(), class_b.value());
}

std::optional<ClassInfo>
ClassTree::common_ancestor(const ClassInfo &class_a,
                           const ClassInfo &class_b) const {
  const ClassInfo *a_side = &class_a;
  const ClassInfo *b_side = &class_b;

  while (a_side->name() != b_side->name()) {
    Symbol superclass;
    ClassIdx idx;

    bool a_is_deeper = a_side->depth() > b_side->depth();

    if (a_is_deeper)
      superclass = a_side->superclass();
    else
      superclass = b_side->superclass();

    if (exists(superclass))
      idx = classes_by_name.at(superclass.id);
    else
      return std::nullopt;

    if (a_is_deeper)
      a_side = &classes[idx];
    else
      b_side = &classes[idx];
  }
  return *a_side;
}

/***********************
 *                     *
 *     TypeContext     *
 *                     *
 **********************/

// TODO(IT) take class hierarchies into consideration
bool TypeContext::match(Symbol type_a, Symbol type_b) const {
  if (type_a == type_b)
    return true;
  if (type_a == symbols.self_type && type_b == current_class)
    return true;

  return false;
}
