#pragma once

#include <cmath>
#include <fstream>
#include <iostream>

#include "../Parser/Parser.h"
#include "../String/String.h"
#include "../Tree/Tree.h"
#include "../UnorderedMap/UnorderedMap.h"
#include "../UnorderedSet/UnorderedSet.h"
#include "../Vector/Vector.h"

#include <texcaller.h>
#include <sstream>

#define Braced(expr) String("(") + expr + String(")")

#define OptimizeBraced(node) \
  (node.is_simple_ ? node.expr_ : "(" + node.expr_ + ")")

#define LaTeXOptimizeBraced(node) \
  (node.is_simple_ ? node.expr_ : "\\left(" + node.expr_ + "\\right)")

#define LaTeXBraced(expr) "\\left(" + expr + "\\right)"

#define PLUS(expr_1, expr_2) expr_1 + "+" + expr_2

#define MINUS(expr_1, expr_2) expr_1 + "-" + expr_2

#define MULT(expr_1, expr_2) expr_1 + "*" + expr_2

#define DIV(expr_1, expr_2) expr_1 + "/" + expr_2

#define LaTeXDIV(expr_1, expr_2) "\\frac{" + expr_1 + "}{" + expr_2 + "}"

#define POW(expr, pow) expr + "^" + pow

#define LaTeXPOW(expr, pow) expr + "^{" + pow + "}"

#define LOG(arg) String("log(") + arg + ")"

#define COS(arg) String("cos(") + arg + ")"

#define SIN(arg) String("sin(") + arg + ")"

#define LaTeXLOG(arg) String("\\ln{") + arg + "}"

#define LaTeXCOS(arg) String("\\cos{") + arg + "}"

#define LaTeXSIN(arg) String("\\sin{") + arg + "}"

#define ZERO String("0")

class Formula {
 public:
  explicit Formula(String expression) {
    auto result = parser_.Parse(std::move(expression));
    if (result) {
      tree_ = result.value();
    }
  }

  String ToString() {
    using StringTree = Tree<StringTreeNode>;
    StringTree string_tree = StringTree::CreateLike(
        tree_, [this](const Parser::ParseTree::PostOrderIterator &formula_iter,
                      StringTree::PostOrderIterator &string_iter) {
          switch (formula_iter->value_->type_) {
            case Parser::BaseTokenTypes::PLUS: {
              const auto &left = string_iter->children_[0];
              const auto &right = string_iter->children_[1];

              string_iter->value_.expr_ =
                  PLUS(left->value_.expr_, right->value_.expr_);
              string_iter->value_.is_simple_ = false;
            } break;

            case Parser::BaseTokenTypes::MINUS: {
              const auto &left = string_iter->children_[0];
              const auto &right = string_iter->children_[1];

              string_iter->value_.expr_ =
                  MINUS(left->value_.expr_, right->value_.expr_);
              string_iter->value_.is_simple_ = false;
            } break;

            case Parser::BaseTokenTypes::MULT: {
              const auto &left = string_iter->children_[0];
              const auto &right = string_iter->children_[1];

              string_iter->value_.expr_ = MULT(OptimizeBraced(left->value_),
                                               OptimizeBraced(right->value_));
              string_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::DIV: {
              const auto &left = string_iter->children_[0];
              const auto &right = string_iter->children_[1];

              string_iter->value_.expr_ = DIV(OptimizeBraced(left->value_),
                                              OptimizeBraced(right->value_));
              string_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::POW: {
              const auto &left = string_iter->children_[0];
              const auto &right = string_iter->children_[1];

              string_iter->value_.expr_ = POW(OptimizeBraced(left->value_),
                                              OptimizeBraced(right->value_));
              string_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::LOG: {
              const auto &arg = string_iter->children_[0];

              string_iter->value_.expr_ = LOG(arg->value_.expr_);
              string_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::SIN: {
              const auto &arg = string_iter->children_[0];

              string_iter->value_.expr_ = SIN(arg->value_.expr_);
              string_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::COS: {
              const auto &arg = string_iter->children_[0];

              string_iter->value_.expr_ = COS(arg->value_.expr_);
              string_iter->value_.is_simple_ = true;
            } break;

            default: {
              string_iter->value_.expr_ = formula_iter->value_->str_;
              string_iter->value_.is_simple_ = true;
            }
          }
        });

    return string_tree.GetRoot()->value_.expr_;
  }

  void ToPDF(const String &filename) {
    std::ofstream out("tmp.tex");

    out << "\\documentclass{article}\n"
           "\\usepackage[T1,T2A]{fontenc}\n"
           "\\usepackage[utf8]{inputenc}\n"
           "\\usepackage[english,russian]{babel}\n"
           "\\usepackage{amsmath}\n"
           "\\begin{document}\n"
           "\\[\n"
           "\\boxed{";

    out << GetLaTeX();

    out << "}\n"
           "\\]\n"
           "\\begin{center}"
           "Утрем нос Стивену Вольфраму!(нет)"
           "\\end{center}"
           "\\end{document}";

    out.close();

    texcaller_to_pdf("LaTeX", "PDF", 5, "tmp.tex", filename.c_str());

    std::remove("tmp.tex");
  }

  Formula At(const UnorderedMap<String, String> &variables) const {
    auto tree = Parser::ParseTree::CreateLike(
        tree_, [this, &variables](
                   const Parser::ParseTree::PostOrderIterator &formula_iter,
                   Parser::ParseTree::PostOrderIterator &mapped_formula_iter) {
          switch (formula_iter->value_->type_) {
            case Parser::BaseTokenTypes::VARIABLE: {
              mapped_formula_iter->value_ = formula_iter->value_;

              auto var_iter = variables.find(formula_iter->value_->str_);
              if (var_iter != variables.end()) {
                mapped_formula_iter->value_ =
                    parser_.AddToken({.type_ = Parser::BaseTokenTypes::NUMBER,
                                      .str_ = var_iter->second,
                                      .priority_ = 0,
                                      .operands_number_ = 0,
                                      .is_function = false});
              }
            } break;

            default: {
              mapped_formula_iter->value_ = formula_iter->value_;
            }
          }
        });

    auto result = Formula(std::move(tree));
    result.Optimize();
    return result;
  }

  const Parser::ParseTree &GetTree() const { return tree_; }

  void Optimize() {
    for (auto &&node = tree_.begin(); node != tree_.end(); ++node) {
      if (node->children_.size() == 2) {
        const auto &left = node->children_[0];
        const auto &right = node->children_[1];
        if (left->value_->type_ == Parser::BaseTokenTypes::NUMBER &&
            right->value_->type_ == Parser::BaseTokenTypes::NUMBER) {
          auto result = HandleNumbers(left->value_->str_, right->value_->str_,
                                      node->value_->type_);
          auto token =
              parser_.AddToken({.type_ = Parser::BaseTokenTypes::NUMBER,
                                .str_ = std::move(result),
                                .priority_ = 0,
                                .operands_number_ = 0,
                                .is_function = false});
          tree_.Replace(node, std::make_shared<Parser::ParseTree::Node>(token));

          continue;
        }
      }

      if (node->children_.size() == 1) {
        const auto &arg = node->children_[0];
        if (arg->value_->type_ == Parser::BaseTokenTypes::NUMBER) {
          auto result = HandleNumbers(arg->value_->str_, node->value_->type_);
          auto token =
              parser_.AddToken({.type_ = Parser::BaseTokenTypes::NUMBER,
                                .str_ = std::move(result),
                                .priority_ = 0,
                                .operands_number_ = 0,
                                .is_function = false});
          tree_.Replace(node, std::make_shared<Parser::ParseTree::Node>(token));

          continue;
        }
      }

      switch (node->value_->type_) {
        case Parser::BaseTokenTypes::PLUS: {
          const auto &left = node->children_[0];
          const auto &right = node->children_[1];

          if (left->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(right).GetRoot());
            break;
          }

          if (right->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }
        } break;

        case Parser::BaseTokenTypes::MINUS: {
          const auto &left = node->children_[0];
          const auto &right = node->children_[1];

          if (right->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }
        } break;

        case Parser::BaseTokenTypes::MULT: {
          const auto &left = node->children_[0];
          const auto &right = node->children_[1];

          if (left->value_->str_ == "1") {
            tree_.Replace(node, tree_.ExtractSubTree(right).GetRoot());
            break;
          }

          if (right->value_->str_ == "1") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (left->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (right->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(right).GetRoot());
            break;
          }
        } break;

        case Parser::BaseTokenTypes::DIV: {
          const auto &left = node->children_[0];
          const auto &right = node->children_[1];

          if (left->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (right->value_->str_ == "1") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }
        } break;

        case Parser::BaseTokenTypes::POW: {
          const auto &left = node->children_[0];
          const auto &right = node->children_[1];

          if (left->value_->str_ == "0") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (left->value_->str_ == "1") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (right->value_->str_ == "1") {
            tree_.Replace(node, tree_.ExtractSubTree(left).GetRoot());
            break;
          }

          if (right->value_->str_ == "0") {
            auto token =
                parser_.AddToken({.type_ = Parser::BaseTokenTypes::NUMBER,
                                  .str_ = "1",
                                  .priority_ = 0,
                                  .operands_number_ = 0,
                                  .is_function = false});
            tree_.Replace(node,
                          std::make_shared<Parser::ParseTree::Node>(token));
            break;
          }
        } break;

        default: {
        }
      }
    }
  }

 private:
  struct StringTreeNode {
    String expr_;
    bool is_simple_ = true;
  };

  explicit Formula(Parser::ParseTree tree) : tree_(std::move(tree)) {}

  static String HandleNumbers(const String &left_str, const String &right_str,
                              int operation) {
    auto left = std::stold(left_str);
    auto right = std::stold(right_str);
    std::stringstream stringstream;
    switch (operation) {
      case Parser::BaseTokenTypes::PLUS: {
        stringstream << left + right;
      } break;
      case Parser::BaseTokenTypes::MINUS: {
        stringstream << left - right;
      } break;
      case Parser::BaseTokenTypes::MULT: {
        stringstream << left * right;
      } break;
      case Parser::BaseTokenTypes::DIV: {
        stringstream << left / right;
      } break;
      case Parser::BaseTokenTypes::POW: {
        stringstream << powl(left, right);
      } break;
      default: {
      }
    }

    return stringstream.str();
  }

  static String HandleNumbers(const String &arg_str, int operation) {
    auto arg = std::stold(arg_str);
    std::stringstream stringstream;
    switch (operation) {
      case Parser::BaseTokenTypes::LOG: {
        stringstream << std::log(arg);
      } break;
      case Parser::BaseTokenTypes::SIN: {
        stringstream << std::sin(arg);
      } break;
      case Parser::BaseTokenTypes::COS: {
        stringstream << std::cos(arg);
      } break;
      default: {
      }
    }

    return stringstream.str();
  }

  String GetLaTeX() {
    using LaTeXTree = Tree<StringTreeNode>;
    LaTeXTree latex_tree = LaTeXTree::CreateLike(
        tree_, [this](const Parser::ParseTree::PostOrderIterator &formula_iter,
                      LaTeXTree::PostOrderIterator &latex_iter) {
          switch (formula_iter->value_->type_) {
            case Parser::BaseTokenTypes::PLUS: {
              const auto &left = latex_iter->children_[0];
              const auto &right = latex_iter->children_[1];

              latex_iter->value_.expr_ =
                  PLUS(left->value_.expr_, right->value_.expr_);
              latex_iter->value_.is_simple_ = false;
            } break;

            case Parser::BaseTokenTypes::MINUS: {
              const auto &left = latex_iter->children_[0];
              const auto &right = latex_iter->children_[1];

              latex_iter->value_.expr_ =
                  MINUS(left->value_.expr_, LaTeXOptimizeBraced(right->value_));
              latex_iter->value_.is_simple_ = false;
            } break;

            case Parser::BaseTokenTypes::MULT: {
              const auto &left = latex_iter->children_[0];
              const auto &right = latex_iter->children_[1];

              latex_iter->value_.expr_ =
                  MULT(LaTeXOptimizeBraced(left->value_),
                       LaTeXOptimizeBraced(right->value_));
              latex_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::DIV: {
              const auto &left = latex_iter->children_[0];
              const auto &right = latex_iter->children_[1];

              latex_iter->value_.expr_ =
                  LaTeXDIV(left->value_.expr_, right->value_.expr_);
              latex_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::POW: {
              const auto &left = latex_iter->children_[0];
              const auto &right = latex_iter->children_[1];

              latex_iter->value_.expr_ = LaTeXPOW(
                  LaTeXOptimizeBraced(left->value_), right->value_.expr_);
              latex_iter->value_.is_simple_ = false;
            } break;

            case Parser::BaseTokenTypes::LOG: {
              const auto &arg = latex_iter->children_[0];

              latex_iter->value_.expr_ =
                  LaTeXLOG(LaTeXBraced(arg->value_.expr_));
              latex_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::SIN: {
              const auto &arg = latex_iter->children_[0];

              latex_iter->value_.expr_ =
                  LaTeXSIN(LaTeXBraced(arg->value_.expr_));
              latex_iter->value_.is_simple_ = true;
            } break;

            case Parser::BaseTokenTypes::COS: {
              const auto &arg = latex_iter->children_[0];

              latex_iter->value_.expr_ =
                  LaTeXCOS(LaTeXBraced(arg->value_.expr_));
              latex_iter->value_.is_simple_ = true;
            } break;

            default: {
              latex_iter->value_.expr_ = formula_iter->value_->str_;
              latex_iter->value_.is_simple_ = true;
            }
          }
        });

    return latex_tree.GetRoot()->value_.expr_;
  }

  static Parser parser_;
  Parser::ParseTree tree_;
};

// Parser Formula::parser_ = Parser();

class Differentiator {
 public:
  Differentiator() = default;

  Formula Differentiate(const String &expr, String variable) {
    variable_ = std::move(variable);

    auto formula = Formula(expr);  // TODO: удалить эту строку

    tree_ = Tree<NodeState>::CreateLike(
        formula.GetTree(),
        [this](const Parser::ParseTree::PostOrderIterator &expr_iter,
               Tree<NodeState>::PostOrderIterator &diff_iter) {
          ProcessNode(diff_iter, expr_iter);
        });

    auto result = Formula(tree_.GetRoot()->value_.diff_);
    result.Optimize();
    return result;
  }

 private:
  struct NodeState {
    String normal_;
    String diff_;
  };

  void ProcessNode(Tree<NodeState>::PostOrderIterator &cur_iter,
                   const Parser::ParseTree::PostOrderIterator &expr_iter) {
    auto &current = cur_iter->value_;

    switch (expr_iter->value_->type_) {
      case Parser::BaseTokenTypes::PLUS: {
        const auto &left = cur_iter->children_[0]->value_;
        const auto &right = cur_iter->children_[1]->value_;

        current.diff_ = PLUS(left.diff_, right.diff_);
        current.normal_ = PLUS(left.normal_, right.normal_);
      } break;

      case Parser::BaseTokenTypes::MINUS: {
        const auto &left = cur_iter->children_[0]->value_;
        const auto &right = cur_iter->children_[1]->value_;

        current.diff_ = MINUS(left.diff_, right.diff_);
        current.normal_ = MINUS(left.normal_, right.normal_);
      } break;

      case Parser::BaseTokenTypes::MULT: {
        const auto &left = cur_iter->children_[0]->value_;
        const auto &right = cur_iter->children_[1]->value_;

        current.diff_ = PLUS(MULT(Braced(left.diff_), Braced(right.normal_)),
                             MULT(Braced(right.diff_), Braced(left.normal_)));
        current.normal_ = MULT(Braced(left.normal_), Braced(right.normal_));
      } break;

      case Parser::BaseTokenTypes::DIV: {
        const auto &left = cur_iter->children_[0]->value_;
        const auto &right = cur_iter->children_[1]->value_;

        current.diff_ =
            DIV(Braced(MINUS(MULT(Braced(left.diff_), Braced(right.normal_)),
                             MULT(Braced(right.diff_), Braced(left.normal_)))),
                POW(Braced(right.normal_), "2"));
        current.normal_ = DIV(Braced(left.normal_), Braced(right.normal_));
      } break;

      case Parser::BaseTokenTypes::POW: {
        // (f ^ g)' = f ^ (g - 1) * (g * f' + f * log(f) * g')

        const auto &left = cur_iter->children_[0]->value_;
        const auto &right = cur_iter->children_[1]->value_;

        current.diff_ = MULT(
            POW(Braced(left.normal_), Braced(MINUS(right.normal_, "1"))),
            Braced(PLUS(MULT(Braced(right.normal_), Braced(left.diff_)),
                        MULT(Braced(left.normal_),
                             MULT(LOG(left.normal_), Braced(right.diff_))))));

        current.normal_ = POW(Braced(left.normal_), Braced(right.normal_));
      } break;

      case Parser::BaseTokenTypes::LOG: {
        // log(f)' = f' / f

        const auto &arg = cur_iter->children_[0]->value_;

        current.diff_ = DIV(Braced(arg.diff_), Braced(arg.normal_));
        current.normal_ = LOG(arg.normal_);
      } break;

      case Parser::BaseTokenTypes::SIN: {
        // sin(f)' = f' * cos(f)

        const auto &arg = cur_iter->children_[0]->value_;

        current.diff_ = MULT(Braced(arg.diff_), COS(arg.normal_));
        current.normal_ = SIN(arg.normal_);
      } break;

      case Parser::BaseTokenTypes::COS: {
        // cos(f)' = 0 - f' * sin(f)

        const auto &arg = cur_iter->children_[0]->value_;

        current.diff_ = MINUS(ZERO, MULT(Braced(arg.diff_), SIN(arg.normal_)));
        current.normal_ = COS(arg.normal_);
      } break;

      case Parser::BaseTokenTypes::NUMBER: {
        current.normal_ = expr_iter->value_->str_;
        current.diff_ = "0";
      } break;

      case Parser::BaseTokenTypes::VARIABLE: {
        if (expr_iter->value_->str_ == variable_) {
          current.diff_ = "1";
        } else {
          current.diff_ = "0";
        }
        current.normal_ = expr_iter->value_->str_;
      } break;

      default: {
        current.diff_ = "0";
        current.normal_ = expr_iter->value_->str_;
      }
    }
  }

  Tree<NodeState> tree_;
  String variable_;
};
