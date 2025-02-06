#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "AnalysisConfig.h"
#include "Filter.h"
#include "FilterType.h"
#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

class Waveform {
public:
  struct PulseProperties {
    double baseline;
    double pulseHeight;
    double shortIntegral;
    double longIntegral;
    double PSP;
    int trigger;
  };

  Waveform(const std::vector<double> &samples, double samplingRate, int ch);

  void analyze(const AnalysisConfig &config);
  void applyFilter(FilterType type, int windowSize);

  const std::vector<double> &getRawData() { return rawData; }
  const std::vector<double> &getFilteredData() { return filteredData; }
  const PulseProperties &getProperties() { return properties; }
  bool isFiltered() const { return !filteredData.empty(); }
  int getChannel() const { return channel; }

private:
  std::vector<double> rawData;
  std::vector<double> filteredData;
  double samplingRate;
  PulseProperties properties;
  int channel;

  void calculateBaseline(int numSamples);
  void findTrigger(double threshold, int polarity);
  void calculateIntegrals(int shortGate, int longGate, int polarity);
  void calculatePulseHeight(int polarity);
};

#endif
