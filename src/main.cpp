#include <string>
#include <iostream>
#include <functional>

#include <coco/combix/error.hpp>
#include <coco/combix/parser_traits.hpp>
#include <coco/combix/primitives.hpp>
#include <coco/combix/combinators.hpp>
#include <coco/combix/parser.hpp>
#include <coco/combix/iterator_stream.hpp>
#include <coco/combix/chars.hpp>
#include <coco/combix/lazy_fun.hpp>

#include "ast.hpp"

namespace cbx = coco::combix;

using stream_type = cbx::iterator_stream<std::string::const_iterator>;

cbx::parser<expr, stream_type> expression();

cbx::parser<expr, stream_type> number() {
  return cbx::expected(cbx::map(cbx::many1(cbx::digit()),
                                [](auto&& is) {
                                  int acc = 0;
                                  for (auto i : is) {
                                    acc = acc * 10 + i;
                                  }
                                  return expr(std::make_shared<integer>(acc));
                                }),
                       "integer number");
}

cbx::parser<expr, stream_type> factor() {
  auto paren_inner = 
      cbx::between(cbx::skip(cbx::token('('), cbx::spaces()),
                   cbx::skip(cbx::token(')'), cbx::spaces()),
                   cbx::skip(cbx::lazy_fun(expression), cbx::spaces()));

  auto paren = cbx::map(
      paren_inner,
      [](auto&& c) { return expr(std::make_shared<parens>(std::move(c))); });

  return cbx::choice(number(), paren);
}

cbx::parser<expr, stream_type> term() {
  auto op = cbx::map(
      cbx::skip(cbx::choice(cbx::token('*'), cbx::token('/')), cbx::spaces()),
      [](auto c) -> std::function<expr(expr, expr)> {
        if (c == '*') {
          return [](auto&& lhs, auto&& rhs) {
            return std::make_shared<binop>(op::MULTI, std::move(lhs),
                                           std::move(rhs));
          };
        } else {
          return [](auto&& lhs, auto&& rhs) {
            return std::make_shared<binop>(op::DIV, std::move(lhs),
                                           std::move(rhs));
          };
        }
      });
  return cbx::chainl1(cbx::skip(factor(), cbx::spaces()), op);
}

cbx::parser<expr, stream_type> expression() {
  auto op = cbx::map(
      cbx::skip(cbx::choice(cbx::token('+'), cbx::token('-')), cbx::spaces()),
      [](auto c) -> std::function<expr(expr, expr)> {
        if (c == '+') {
          return [](auto&& lhs, auto&& rhs) {
            return std::make_shared<binop>(op::PLUS, std::move(lhs),
                                           std::move(rhs));
          };
        } else {
          return [](auto&& lhs, auto&& rhs) {
            return std::make_shared<binop>(op::MINUS, std::move(lhs),
                                           std::move(rhs));
          };
        }
      });
  return cbx::chainl1(cbx::skip(term(), cbx::spaces()), op);
}

struct calculator {
  int operator()(integer_ptr const& p) const {
    return p->value;
  }

  int operator()(binop_ptr const& p) const {
    auto l = boost::apply_visitor(*this, p->lhs);
    auto r = boost::apply_visitor(*this, p->rhs);
    switch (p->kind) {
    case op::PLUS:
      return l + r;
    case op::MINUS:
      return l - r;
    case op::MULTI:
      return l * r;
    case op::DIV:
      return l / r;
    }
  }

  int operator()(parens_ptr const& p) const {
    return boost::apply_visitor(*this, p->body);
  }
};

int main() {
  std::string src;
  std::getline(std::cin, src);
  auto n = number();
  auto stream = cbx::range_stream(src);
  auto const parser = expression();
  if (auto res = cbx::parse(parser, stream)) {
    std::cout << to_string(*res) << std::endl;
    std::cout << "Result: " << boost::apply_visitor(calculator(), *res)
              << std::endl;
  } else {
    std::cout << cbx::to_string(res.unwrap_error()) << std::endl;
  }
}
