#ifndef FILTER_H
#define FILTER_H

#include "FilterType.h"
#include <eigen3/Eigen/Dense>
#include <numeric>
#include <vector>

class Filter {
public:
  static std::vector<double> movingAverage(const std::vector<double> &data,
                                           int windowSize);
  static std::vector<double> savitzkyGolay(const std::vector<double> &data,
                                           int windowSize, int order);
};

#endif
