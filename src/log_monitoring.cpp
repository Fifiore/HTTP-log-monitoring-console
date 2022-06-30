#include <iostream>
#include <string>
#include "monitoring.h"

inline const std::string THRESHOLD_PARAM = "--alert_th=";

bool is_threshold_param(std::string param) {
  return param.size() > THRESHOLD_PARAM.size() && 0 == param.find(THRESHOLD_PARAM);
}

int get_threshold_param(std::string param) {
  int threshold =0;
  try {
    threshold = std::stoi(param.substr(THRESHOLD_PARAM.size()));
  }
  catch (const std::invalid_argument) { }
  if (0 >= threshold) {
    throw std::invalid_argument("Invalid thresold parameter");
  }
  return threshold;
}

int main(int argc, char **argv) {
  
  try {
    if (argc > 3) {
      throw std::invalid_argument("Too many parameters");
    }

    int threshold = 0;
    std::string inputFile = "-";

    if (argc > 1) {
      std::string param1 = argv[1];

      if (is_threshold_param(param1)) {
        threshold = get_threshold_param(param1);
      }
      else
        inputFile = param1;

      if (argc == 3) {
        std::string param2 = argv[2];
        if (threshold) {
          inputFile = param2;
        }
        else if (is_threshold_param(param2)) {
          threshold = get_threshold_param(param2);
        }
        else {
          throw std::invalid_argument("Invalid parameter");
        }
      }
    }
    monitoring::execute(inputFile, threshold);
  }
  catch (const std::exception & ex) {
    std::cout << ex.what() << std::endl;
  }
}
