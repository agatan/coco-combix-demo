#include <string>
#include <iostream>
#include <functional>

#include <coco/combix/parser_traits.hpp>
#include <coco/combix/primitives.hpp>
#include <coco/combix/combinators.hpp>
#include <coco/combix/parser.hpp>
#include <coco/combix/iterator_stream.hpp>
#include <coco/combix/chars.hpp>
#include <coco/combix/lazy_fun.hpp>

namespace cbx = coco::combix;

using stream_type = cbx::iterator_stream<std::string::const_iterator>;

cbx::parser<int, stream_type> expression();

cbx::parser<int, stream_type> number() {
  return cbx::map(cbx::many1(cbx::digit()), [](auto&& is) {
    int acc = 0;
    for (auto i : is) {
      acc = acc * 10 + i;
    }
    return acc;
  });
}

cbx::parser<int, stream_type> factor() {
  return cbx::choice(number(), cbx::parens(cbx::lazy_fun(expression)));
}

cbx::parser<int, stream_type> term() {
  auto op = cbx::map(cbx::choice(cbx::token('*'), cbx::token('/')),
                     [](auto c) -> std::function<int(int, int)> {
                       if (c == '*') {
                         return std::multiplies<int>();
                       } else {
                         return std::divides<int>();
                       }
                     });
  return cbx::chainl1(factor(), op);
}

cbx::parser<int, stream_type> expression() {
  auto op = cbx::map(cbx::choice(cbx::token('+'), cbx::token('-')),
                     [](auto c) -> std::function<int(int, int)> {
                       if (c == '+') {
                         return std::plus<int>();
                       } else {
                         return std::minus<int>();
                       }
                     });
  return cbx::chainl1(term(), op);
}

int main() {
  std::string const src = "(1+2)*3";
  auto n = number();
  auto stream = cbx::range_stream(src);
  auto const parser = expression();
  std::cout << cbx::parse(parser, stream).unwrap() << std::endl;
}
