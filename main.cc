#include <cstdio>
#include <cstring>
#include <print>
#include <tuple>
#include <vector>

/*
WIP proof of concept parser combinator stuff
*/

using usize = std::size_t;

struct parser_result {
  bool result;
  const char *consume;

  operator std::pair<bool, const char *>() {
    return std::pair{result, consume};
  }

  operator std::tuple<bool, const char *>() {
    return std::tuple{result, consume};
  }
};

constexpr static auto consume = [](auto parser, auto &pointer) {
  parser_result result = parser(pointer);
  if (result.result) {
    pointer = result.consume;
  }
  return result.result;
};

constexpr static auto any = [](auto... parsers) {
  return [=](const char *_ipt) -> parser_result {
    return {(consume(parsers, _ipt) || ...), _ipt};
  };
};

constexpr static auto all = [](auto... parsers) {
  return [=](const char *_ipt) -> parser_result {
    return {(consume(parsers, _ipt) && ...), _ipt};
  };
};

constexpr static auto eq = [](char _testcase) {
  return [=](const char *_ipt) -> parser_result {
    return {_testcase == *_ipt, _ipt + 1};
  };
};

constexpr static auto is_alpha = []() {
  return [=](const char *_ipt) -> parser_result {
    return {(bool)std::isalpha(*_ipt), _ipt + 1};
  };
};

constexpr static auto is_alnum = []() {
  return [=](const char *_ipt) -> parser_result {
    return {(bool)std::isalnum(*_ipt), _ipt + 1};
  };
};

constexpr static auto is_digit = []() {
  return [=](const char *_ipt) -> parser_result {
    return {(bool)std::isdigit(*_ipt), _ipt + 1};
  };
};

constexpr static auto is_nonzero_digit = []() {
  return [=](const char *_ipt) -> parser_result {
    return {(bool)std::isdigit(*_ipt) && *_ipt != '0', _ipt + 1};
  };
};

constexpr static auto is_space = []() {
  return [=](const char *_ipt) -> parser_result {
    return {(bool)std::isspace(*_ipt), _ipt + 1};
  };
};

/* equivalent to (...)* in regular expressions */
constexpr static auto many = [](auto parser) {
  return [=](const char *_ipt) -> parser_result {
    while (consume(parser, _ipt))
      ;
    return {true, _ipt};
  };
};

constexpr static auto more = [](auto parser) {
  return [=](const char *_ipt) -> parser_result {
    auto first_result = parser(_ipt).result;
    if (!first_result) {
      return {false, _ipt};
    } else {
      _ipt++;
      while (consume(parser, _ipt))
        ;
      return {true, _ipt};
    }
  };
};

constexpr static auto annotate = [](auto parser, auto annotation) {
  return [=](const char *_ipt) {
    const char *const original_pointer = _ipt;
    parser_result result = parser(_ipt);
    if (result.result) {
      annotation(std::string(original_pointer,
                             (usize)(result.consume - original_pointer)));
    }
    return result;
  };
};

/*

we need many for this

(a*b)+

*/

enum class e_token_type {
  identifier,
  lparen,
  rparen,
  lbrack,
  rbrack,
  int_literal,
  float_literal,
  double_literal,
  str_literal,
  arithmetic_operator,
  semicolon,
};

struct lexer_token {
  e_token_type type;
  std::string value;

  void dbg_print() {
    switch (type) {
    case e_token_type::identifier:
      std::print("TOKEN IDENTIFIER {}\n", value.c_str());
      break;
    case e_token_type::lparen:
      std::print("TOKEN LPAREN\n");
      break;
    case e_token_type::rparen:
      std::print("TOKEN RPAREN\n");
      break;
    case e_token_type::lbrack:
      std::print("TOKEN LBRACK\n");
      break;
    case e_token_type::rbrack:
      std::print("TOKEN RBRACK\n");
      break;
    case e_token_type::int_literal:
      std::print("TOKEN INT LITERAL {}\n", value.c_str());
      break;
    case e_token_type::float_literal:
      std::print("TOKEN FLOAT LITERAL {}\n", value.c_str());
      break;
    case e_token_type::double_literal:
      std::print("TOKEN DOUBLE LITERAL {}\n", value.c_str());
      break;
    case e_token_type::str_literal:
      std::print("TOKEN STR LITERAL {}\n", value.c_str());
      break;
    case e_token_type::arithmetic_operator:
      std::print("TOKEN MATH OP {}\n", value.c_str());
      break;
    case e_token_type::semicolon:
      std::print("TOKEN SEMICOLON\n");
      break;
    }
  }
};

int main() {
  std::vector<lexer_token> tokens;

  auto identifier = annotate(
      all(more(any(is_alpha(), eq('_'))), many(any(is_alnum(), eq('_')))),
      [&](std::string _token_value) {
        tokens.push_back({e_token_type::identifier, _token_value});
      });

  auto lparen = annotate(eq('('), [&](std::string _token_value) {
    tokens.push_back({e_token_type::lparen, "{"});
  });

  auto rparen = annotate(eq(')'), [&](std::string _token_value) {
    tokens.push_back({e_token_type::rparen, ")"});
  });

  auto lbrack = annotate(eq('{'), [&](std::string _token_value) {
    tokens.push_back({e_token_type::lbrack, "{"});
  });

  auto rbrack = annotate(eq('}'), [&](std::string _token_value) {
    tokens.push_back({e_token_type::rbrack, "}"});
  });

  auto int_literal =
      annotate(any(eq('0'), all(more(is_nonzero_digit()), many(is_digit()))),
               [&](std::string _token_value) {
                 tokens.push_back({e_token_type::int_literal, _token_value});
               });

  auto float_literal =
      annotate(all(any(eq('0'), all(is_nonzero_digit(), many(is_digit()))),
                   eq('.'), many(is_digit()), any(eq('f'), eq('F'))),
               [&](std::string _token_value) {
                 tokens.push_back({e_token_type::float_literal, _token_value});
               });

  auto double_literal =
      annotate(all(any(eq('0'), all(is_nonzero_digit(), many(is_digit()))),
                   eq('.'), many(is_digit())),
               [&](std::string _token_value) {
                 tokens.push_back({e_token_type::double_literal, _token_value});
               });

  auto arithmetic_operator = annotate(
      any(eq('+'), eq('-'), eq('/'), eq('*'), eq('=')),
      [&](std::string _token_value) {
        tokens.push_back({e_token_type::arithmetic_operator, _token_value});
      });

  auto semicolon = annotate(eq(';'), [&](std::string _token_value) {
    tokens.push_back({e_token_type::semicolon, _token_value});
  });

  /*
  make sure ambiguous lexemes are sorted (might not work) -> find solution
  */
  auto program = all(many(any(is_space(), identifier, lparen, rparen, lbrack,
                              rbrack, double_literal, float_literal,
                              int_literal, arithmetic_operator, semicolon)),
                     many(is_space()), eq('\0'));

  std::string input =
      R"(
    
int main() {

    printf(hello);
    int b = 1 + 3 + 1.06f;
    return 0;

}


)";

  std::print("{}\n", program(input.c_str()).result);

  for (auto &token : tokens) {
    token.dbg_print();
  }
}
