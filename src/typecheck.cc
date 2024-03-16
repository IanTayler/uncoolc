#include "error.h"
#include "semantic.h"

/***********************
 *                     *
 *     Basic Nodes     *
 *                     *
 **********************/

bool ExpressionNode::typecheck(TypeContext context) {
  fatal("Calling typecheck on an abstract ExpressionNode is not permitted",
        start_token);
  return false; // fool the linters
}

// TODO(IT) fill in
bool AttributeNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool ParameterNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool MethodNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool ClassNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool ModuleNode::typecheck(TypeContext context) { return true; }

/***********************
 *                     *
 *  Atomic Expressions *
 *                     *
 **********************/

// TODO(IT) fill in
bool LiteralNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool VariableNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in

/***********************
 *                     *
 *  Simple Operations  *
 *                     *
 **********************/

bool UnaryOpNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool BinaryOpNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool NewNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool AssignNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool DispatchNode::typecheck(TypeContext context) { return true; }

/***********************
 *                     *
 *  Complex Structures *
 *                     *
 **********************/

// TODO(IT) fill in
bool BlockNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool IfNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool WhileNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool LetNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool CaseBranchNode::typecheck(TypeContext context) { return true; }
// TODO(IT) fill in
bool CaseNode::typecheck(TypeContext context) { return true; }
