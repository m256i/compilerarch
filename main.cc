#include <cstdio>
#include <cstring>
#include <iostream>
#include <print>
#include <tuple>
#include <vector>

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

// template <typename TType, typename TFunc, std::size_t... TIndex>
// constexpr void
// visit_impl(TType& _tuple, const size_t _index, TFunc _func,
// std::index_sequence<TIndex...>)
// {
//   ((TIndex == _index ? _func(std::get<TIndex>(_tuple)) : void()), ...);
// }
// template <typename TFunc, typename... TTypes, typename _Indices =
// std::make_index_sequence<sizeof...(TTypes)>> constexpr void
// visit_at(std::tuple<TTypes...>& _tuple, const size_t _index, TFunc _func)
// {
//   visit_impl(_tuple, _index, _func, _Indices{});
// }
// template <typename TFunc, typename... TTypes, typename _Indices =
// std::make_index_sequence<sizeof...(TTypes)>> constexpr void visit_at(const
// std::tuple<TTypes...>& _tuple, const size_t _index, TFunc _func)
// {
//   visit_impl(_tuple, _index, _func, _Indices{});
// }
// constexpr static auto match = [](auto... parsers) {
//     std::tuple parser_tuple{parsers...};
//     return [=](const char* _ipt) -> parser_result {
//         std::vector<const char*> ipts(sizeof...(parsers), _ipt);
//         std::vector<parser_result> results{};
//         parser_result final_result{false, _ipt};
//         for (usize i = 0; i != sizeof...(parsers); ++i) {
//             visit_at (parser_tuple, i,
//                 [&] (auto &_arg) {
//                     using type = std::decay_t<decltype (_arg)>;
//                     bool result = consume(_arg, ipts[i]);
//                     if (result) {
//                         results.push_back({true, ipts[i]});
//                     }
//                 });
//         }
//         const char* max_progress = 0;
//         for (const auto p : results) {
//             if (p.consume > max_progress) {
//                 final_result = p;
//                 max_progress = p.consume;
//             }
//         }
//         return final_result;
//     };
// };

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

  auto program = all(many(any(is_space(), identifier, lparen, rparen, lbrack,
                              rbrack, float_literal, double_literal,
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