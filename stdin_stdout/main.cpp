#include <Differentiator.h>
#include <iostream>

void Dialog(const Formula& formula) {
  std::string answer;
  while (true) {
    std::cout << "Want to see the value of the function at a point? (y/n)"
              << std::endl;
    std::cin >> answer;
    if (answer == "y" || answer == "Y") {
      std::cout << "Type up the function values in the following format: "
                   "[variable value], complete with [0 0]"
                << std::endl;
      std::string variable;
      std::string value;
      UnorderedMap<String, String> variables_;
      while (true) {
        std::cin >> variable >> value;
        if (variable == "0" && value == "0") {
          break;
        }
        variables_.insert({variable, value});
      }

      std::cout << "Function value is " + formula.At(variables_).ToString()
                << std::endl
                << std::endl;
    } else {
      break;
    }
  }
}

int main(int argc, char** argv) {
  Differentiator differentiator;
  auto formula = differentiator.Differentiate(argv[1], argv[2]);
  formula.ToPDF(argv[3]);
  std::cout << formula.ToString() << std::endl;
  Dialog(formula);
  return 0;
}