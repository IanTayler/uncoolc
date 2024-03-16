#include "semantic.h"
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
