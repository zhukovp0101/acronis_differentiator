#pragma once

#include <cassert>
#include <memory>

#include "../String/String.h"
#include "../Tree/Tree.h"
#include "../UnorderedMap/UnorderedMap.h"
#include "../UnorderedSet/UnorderedSet.h"
#include "../Vector/Vector.h"

class Parser {
 public:
  class BaseTokenTypes {
   public:
    static const int LBRACE = 0;
    static const int RBRACE = 1;
    static const int PLUS = 2;
    static const int MINUS = 3;
    static const int MULT = 4;
    static const int DIV = 5;
    static const int POW = 6;
    static const int NUMBER = 7;
    static const int VARIABLE = 8;
    static const int LOG = 9;
    static const int SIN = 10;
    static const int COS = 11;
  };
  struct Token {
    int type_;
    String str_;
    size_t priority_;
    size_t operands_number_;
    bool is_function = false;
  };

  class TokenRef {
   public:
    TokenRef() = default;
    TokenRef(Parser *owner, size_t id) : id_(id), owner_(owner) {}
    Token &operator*() { return owner_->tokens_[id_]; }

    Token *operator->() { return &owner_->tokens_[id_]; }

    const Token &operator*() const { return owner_->tokens_[id_]; }

    const Token *operator->() const { return &owner_->tokens_[id_]; }

   private:
    size_t id_ = 0;
    Parser *owner_ = nullptr;
  };

  using ParseTree = Tree<TokenRef>;

  Parser() { BaseInitialize(); }

  TokenRef AddToken(Token token) {
    tokens_.push_back(std::move(token));
    TokenRef token_ref(this, tokens_.size() - 1);
    tokens_refs_.insert({tokens_.back().str_, token_ref});
    return token_ref;
  }

  void AddDelimiter(char delimiter) { delimiters_.insert({delimiter, Unit()}); }

  std::optional<ParseTree> Parse(String expr) {
    expr_ = std::move(expr);
    position_ = 0;
    stack_ = {};
    token_stack_ = {};

    bool parsing_error = false;
    while (auto token = Iterate(parsing_error)) {
      ProcessToken(token.value());
    }

    if (parsing_error) {
      return {};
    }

    while (!token_stack_.empty()) {
      if (0 != MoveTokenFromStack()) {
        return {};
      }
    }

    if (stack_.size() != 1) {
      return {};
    }

    return ParseTree(stack_.front());
  }

 private:
  void BaseInitialize() {
    {
      AddToken({.type_ = BaseTokenTypes::LBRACE,
                .str_ = "(",
                .priority_ = 0,
                .operands_number_ = 0,
                .is_function = false});
      AddToken({.type_ = BaseTokenTypes::RBRACE,
                .str_ = ")",
                .priority_ = 0,
                .operands_number_ = 0,
                .is_function = false});

      AddToken({.type_ = BaseTokenTypes::PLUS,
                .str_ = "+",
                .priority_ = 1,
                .operands_number_ = 2,
                .is_function = false});
      AddToken({.type_ = BaseTokenTypes::MINUS,
                .str_ = "-",
                .priority_ = 1,
                .operands_number_ = 2,
                .is_function = false});

      AddToken({.type_ = BaseTokenTypes::MULT,
                .str_ = "*",
                .priority_ = 2,
                .operands_number_ = 2,
                .is_function = false});
      AddToken({.type_ = BaseTokenTypes::DIV,
                .str_ = "/",
                .priority_ = 2,
                .operands_number_ = 2,
                .is_function = false});

      AddToken({.type_ = BaseTokenTypes::POW,
                .str_ = "^",
                .priority_ = 3,
                .operands_number_ = 2,
                .is_function = false});
      AddToken({.type_ = BaseTokenTypes::LOG,
                .str_ = "log",
                .priority_ = 4,
                .operands_number_ = 1,
                .is_function = true});
      AddToken({.type_ = BaseTokenTypes::SIN,
                .str_ = "sin",
                .priority_ = 4,
                .operands_number_ = 1,
                .is_function = true});
      AddToken({.type_ = BaseTokenTypes::COS,
                .str_ = "cos",
                .priority_ = 4,
                .operands_number_ = 1,
                .is_function = true});
    }

    {
      AddDelimiter(' ');
      AddDelimiter(',');
    }
  }

  int MoveTokenFromStack() {
    if (token_stack_.empty()) {
      return -1;
    }

    if (token_stack_.back()->str_ == "(") {
      token_stack_.pop_back();
      return 1;
    }

    ParseTree::Node::Ptr new_node =
        std::make_shared<ParseTree::Node>(token_stack_.back());
    token_stack_.pop_back();

    size_t operands_number = new_node->value_->operands_number_;
    assert(operands_number > 0);

    if (stack_.size() < operands_number) {
      return -1;
    }

    for (size_t i = operands_number; i > 0; --i) {
      ParseTree::Node::Attach(new_node, stack_[stack_.size() - i]);
    }

    stack_.resize(stack_.size() - operands_number);
    stack_.push_back(new_node);

    return 0;
  }

  int ProcessToken(TokenRef token_ref) {
    if (token_ref->str_ == "(") {
      token_stack_.push_back(token_ref);
      return 0;
    }

    if (token_ref->str_ == ")") {
      do {
        int res = MoveTokenFromStack();
        if (res == -1) {
          return -1;
        }
        if (res == 1) {
          return 0;
        }
      } while (true);
    }

    if (token_ref->priority_ == 0) {
      stack_.push_back(std::make_shared<ParseTree::Node>(token_ref));
    } else {
      while (!token_stack_.empty() &&
             token_ref->priority_ <= token_stack_.back()->priority_) {
        if (MoveTokenFromStack() != 0) {
          return -1;
        }
      }
      token_stack_.push_back(token_ref);
    }

    return 0;
  }

  std::optional<TokenRef> TryNumberOrVariableIteration() {
    String partial_token;
    int type = BaseTokenTypes::NUMBER;
    while (position_ < expr_.size() && '0' <= expr_[position_] &&
           expr_[position_] <= '9') {
      partial_token += expr_[position_++];
    }

    if (partial_token.empty()) {
      type = BaseTokenTypes::VARIABLE;
      while (position_ < expr_.size() && 'a' <= expr_[position_] &&
             expr_[position_] <= 'z') {
        partial_token += expr_[position_++];
      }
    }

    if (partial_token.empty()) {
      return {};
    }

    auto token_iter = tokens_refs_.find(partial_token);
    if (token_iter != tokens_refs_.end()) {
      return token_iter->second;
    }

    return AddToken({.type_ = type,
                     .str_ = partial_token,
                     .priority_ = 0,
                     .operands_number_ = 0,
                     .is_function = false});
  }

  std::optional<TokenRef> Iterate(bool &error) {
    String partial_token;
    while (position_ < expr_.size() &&
           delimiters_.find(expr_[position_]) != delimiters_.end()) {
      ++position_;
    }

    auto number_or_variable = TryNumberOrVariableIteration();
    if (number_or_variable) {
      return number_or_variable;
    }

    while (position_ < expr_.size()) {
      partial_token += expr_[position_++];

      const auto &token_iter = tokens_refs_.find(partial_token);
      if (token_iter != tokens_refs_.end()) {
        return token_iter->second;
      }
    }

    if (!partial_token.empty()) {
      error = true;
    }

    return {};
  }

 private:
  UnorderedMap<String, TokenRef> tokens_refs_;
  Vector<Token> tokens_;
  UnorderedSet<char> delimiters_;
  String expr_;
  size_t position_ = 0;
  Vector<ParseTree::Node::Ptr> stack_{};
  Vector<TokenRef> token_stack_;
};
