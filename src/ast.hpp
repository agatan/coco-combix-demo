#ifndef AST_HPP_
#define AST_HPP_

#include <memory>
#include <string>

#include <boost/variant.hpp>

struct integer;
using integer_ptr = std::shared_ptr<integer>;
struct binop;
using binop_ptr = std::shared_ptr<binop>;
struct parens;
using parens_ptr = std::shared_ptr<parens>;

using expr = boost::variant<integer_ptr, binop_ptr, parens_ptr>;

struct integer {
  int value;

  integer() = default;
  explicit integer(int v) : value(v) {}
};

enum class op {
  PLUS,
  MINUS,
  MULTI,
  DIV,
};

struct binop {
  op kind;
  expr lhs;
  expr rhs;

  binop() = default;
  binop(op k, expr l, expr r) : kind(k), lhs(std::move(l)), rhs(std::move(r)) {}
};

struct parens {
  expr body;

  parens() = default;
  explicit parens(expr body) : body(std::move(body)) {}
};

std::string to_string(expr const& e) {
  struct visitor {
    std::string operator()(integer_ptr const& p) const {
      return std::to_string(p->value);
    }

    std::string operator()(binop_ptr const& p) const {
      auto l = to_string(p->lhs);
      auto r = to_string(p->rhs);
      switch (p->kind) {
      case op::PLUS:
        return l + " + " + r;
      case op::MINUS:
        return l + " - " + r;
      case op::MULTI:
        return l + " * " + r;
      case op::DIV:
        return l + " / " + r;
      }
    }

    std::string operator()(parens_ptr const& p) const {
      return "(" + to_string(p->body) + ")";
    }
  };

  return boost::apply_visitor(visitor{}, e);
}

#endif
