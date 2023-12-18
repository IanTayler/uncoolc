#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "symbol.h"

TEST_SUITE("Symbol") {
  TEST_CASE("Symbol creation") {
    CHECK(Symbol() == Symbol());
    CHECK(Symbol(1) == Symbol(1));
    CHECK(Symbol(0) != Symbol(1));
    CHECK(Symbol(0) != Symbol());
    CHECK(Symbol(10) == Symbol(10));
    CHECK(Symbol(1) == Symbol(1));
    CHECK(Symbol(20000) == Symbol(20000));
  }

  TEST_CASE("Symbol is_empty") {
    CHECK(Symbol().is_empty());
    CHECK(!Symbol(0).is_empty());
    CHECK(!Symbol(1).is_empty());
    CHECK(!Symbol(200).is_empty());
    CHECK(!Symbol(20000).is_empty());
  }
}

bool not_builtin_symbol(Symbol s, SymbolTable *st) {
  return (s != st->true_const && s != st->false_const && s != st->self_var &&
          s != st->self_type && s != st->object_type && s != st->io_type &&
          s != st->bool_type && s != st->int_type && s != st->string_type &&
          s != st->add_op && s != st->sub_op && s != st->div_op &&
          s != st->mult_op && s != st->leq_op && s != st->lt_op &&
          s != st->eq_op && s != st->assign_op && s != st->neg_op &&
          s != st->if_kw && s != st->in_kw && s != st->fi_kw &&
          s != st->of_kw && s != st->let_kw && s != st->new_kw &&
          s != st->not_kw && s != st->case_kw && s != st->else_kw &&
          s != st->esac_kw && s != st->then_kw && s != st->loop_kw &&
          s != st->pool_kw && s != st->while_kw && s != st->class_kw &&
          s != st->isvoid_kw && s != st->inherits_kw);
}

TEST_SUITE("SymbolTable") {
  TEST_CASE("SymbolTable named symbol creation") {
    auto st = SymbolTable();
    CHECK(st.from("true") == st.true_const);
    CHECK(st.from("false") == st.false_const);
    CHECK(st.from("self") == st.self_var);
    CHECK(st.from("SELF_TYPE") == st.self_type);
    CHECK(st.from("Object") == st.object_type);
    CHECK(st.from("IO") == st.io_type);
    CHECK(st.from("String") == st.string_type);
    CHECK(st.from("+") == st.add_op);
    CHECK(st.from("-") == st.sub_op);
    CHECK(st.from("/") == st.div_op);
    CHECK(st.from("*") == st.mult_op);
    CHECK(st.from("<=") == st.leq_op);
    CHECK(st.from("<") == st.lt_op);
    CHECK(st.from("=") == st.eq_op);
    CHECK(st.from("<-") == st.assign_op);
    CHECK(st.from("if") == st.if_kw);
    CHECK(st.from("in") == st.in_kw);
    CHECK(st.from("fi") == st.fi_kw);
    CHECK(st.from("of") == st.of_kw);
    CHECK(st.from("let") == st.let_kw);
    CHECK(st.from("new") == st.new_kw);
    CHECK(st.from("not") == st.not_kw);
    CHECK(st.from("case") == st.case_kw);
    CHECK(st.from("else") == st.else_kw);
    CHECK(st.from("esac") == st.esac_kw);
    CHECK(st.from("then") == st.then_kw);
    CHECK(st.from("loop") == st.loop_kw);
    CHECK(st.from("pool") == st.pool_kw);
    CHECK(st.from("while") == st.while_kw);
    CHECK(st.from("class") == st.class_kw);
    CHECK(st.from("isvoid") == st.isvoid_kw);
    CHECK(st.from("inherits") == st.inherits_kw);
  }

  TEST_CASE("SymbolTable from consistency") {
    auto st = SymbolTable();

    CHECK(not_builtin_symbol(Symbol(), &st));
    CHECK(not_builtin_symbol(st.from("nonkeyword"), &st));
    CHECK(not_builtin_symbol(st.from("a"), &st));
    CHECK(not_builtin_symbol(st.from("b"), &st));

    CHECK(st.from("true") == st.from("true"));
    CHECK(st.from("Main") == st.from("Main"));
    CHECK(st.from("Main") != st.from("main"));
    CHECK(st.from("equals") != st.from("main"));
    CHECK(st.from("equals") == st.from("equals"));
    CHECK(st.from("Equals") == st.from("Equals"));
  }

  TEST_CASE("SymbolTable default symbol strings") {
    auto st = SymbolTable();
    CHECK(std::string("true") == st.get_string(st.true_const));
    CHECK(std::string("false") == st.get_string(st.false_const));
    CHECK(std::string("self") == st.get_string(st.self_var));
    CHECK(std::string("SELF_TYPE") == st.get_string(st.self_type));
    CHECK(std::string("Object") == st.get_string(st.object_type));
    CHECK(std::string("IO") == st.get_string(st.io_type));
    CHECK(std::string("String") == st.get_string(st.string_type));
    CHECK(std::string("+") == st.get_string(st.add_op));
    CHECK(std::string("-") == st.get_string(st.sub_op));
    CHECK(std::string("/") == st.get_string(st.div_op));
    CHECK(std::string("*") == st.get_string(st.mult_op));
    CHECK(std::string("<=") == st.get_string(st.leq_op));
    CHECK(std::string("<") == st.get_string(st.lt_op));
    CHECK(std::string("=") == st.get_string(st.eq_op));
    CHECK(std::string("<-") == st.get_string(st.assign_op));
    CHECK(std::string("if") == st.get_string(st.if_kw));
    CHECK(std::string("in") == st.get_string(st.in_kw));
    CHECK(std::string("fi") == st.get_string(st.fi_kw));
    CHECK(std::string("of") == st.get_string(st.of_kw));
    CHECK(std::string("let") == st.get_string(st.let_kw));
    CHECK(std::string("new") == st.get_string(st.new_kw));
    CHECK(std::string("not") == st.get_string(st.not_kw));
    CHECK(std::string("case") == st.get_string(st.case_kw));
    CHECK(std::string("else") == st.get_string(st.else_kw));
    CHECK(std::string("esac") == st.get_string(st.esac_kw));
    CHECK(std::string("then") == st.get_string(st.then_kw));
    CHECK(std::string("loop") == st.get_string(st.loop_kw));
    CHECK(std::string("pool") == st.get_string(st.pool_kw));
    CHECK(std::string("while") == st.get_string(st.while_kw));
    CHECK(std::string("class") == st.get_string(st.class_kw));
    CHECK(std::string("isvoid") == st.get_string(st.isvoid_kw));
    CHECK(std::string("inherits") == st.get_string(st.inherits_kw));
  }
}
