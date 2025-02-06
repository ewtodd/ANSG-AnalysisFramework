#include "Filter.h"

std::vector<double> Filter::movingAverage(const std::vector<double> &data,
                                          int windowSize) {
  std::vector<double> filtered(data.size());

  for (size_t i = 0; i < data.size(); i++) {
    int start = std::max(0, static_cast<int>(i) - windowSize / 2);
    int end = std::min(data.size(), i + windowSize / 2 + 1);

    filtered[i] =
        std::accumulate(data.begin() + start, data.begin() + end, 0.0) /
        (end - start);
  }
  return filtered;
}

std::vector<double> Filter::savitzkyGolay(const std::vector<double> &data,
                                          int windowSize, int order) {
  std::vector<double> filtered(data.size());
  int halfWindow = windowSize / 2;

  // Create Vandermonde matrix
  Eigen::MatrixXd A(windowSize, order + 1);
  for (int i = 0; i < windowSize; i++) {
    for (int j = 0; j <= order; j++) {
      A(i, j) = std::pow(i - halfWindow, j);
    }
  }

  // Compute filter coefficients
  Eigen::VectorXd h =
      (A * (A.transpose() * A).inverse() * A.transpose()).row(halfWindow);

  // Apply filter
  for (size_t i = 0; i < data.size(); i++) {
    int start = std::max(0, static_cast<int>(i) - halfWindow);
    int end = std::min(data.size(), i + halfWindow + 1);

    double sum = 0.0;
    for (int j = start; j < end; j++) {
      sum += data[j] * h[j - start];
    }
    filtered[i] = sum;
  }

  return filtered;
}
