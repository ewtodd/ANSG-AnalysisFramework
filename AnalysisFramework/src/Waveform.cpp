#include "Waveform.h"

Waveform::Waveform(const std::vector<double> &samples, double rate, int ch)
    : rawData(samples), samplingRate(rate), channel(ch) {
  filteredData.reserve(samples.size());
}

void Waveform::calculateBaseline(int numSamples) {
  const std::vector<double> &dataToUse = isFiltered() ? filteredData : rawData;

  if (numSamples > dataToUse.size()) {
    numSamples = dataToUse.size();
  }
  properties.baseline =
      std::accumulate(dataToUse.begin(), dataToUse.begin() + numSamples, 0.0) /
      numSamples;
}

void Waveform::findTrigger(double threshold, int polarity) {
  const std::vector<double> &dataToUse = isFiltered() ? filteredData : rawData;

  auto it = std::find_if(dataToUse.begin(), dataToUse.end(),
                         [threshold, polarity, this](double sample) {
                           return polarity * (sample - properties.baseline) >
                                  threshold;
                         });

  properties.trigger =
      (it != dataToUse.end()) ? std::distance(dataToUse.begin(), it) : -1;
}
void Waveform::applyFilter(FilterType type, int windowSize) {
  switch (type) {
  case FilterType::MovingAverage:
    filteredData = Filter::movingAverage(rawData, windowSize);
    break;

  case FilterType::SavitzkyGolay:
    filteredData = Filter::savitzkyGolay(rawData, windowSize,
                                         3); // 3rd order polynomial
    break;
  }
}

void Waveform::calculatePulseHeight(int polarity) {
  if (properties.trigger < 0) {
    properties.pulseHeight = 0.0;
    return;
  }

  const std::vector<double> &dataToUse = isFiltered() ? filteredData : rawData;

  auto extremeElement =
      (polarity > 0) ? std::max_element(dataToUse.begin() + properties.trigger,
                                        dataToUse.end())
                     : std::min_element(dataToUse.begin() + properties.trigger,
                                        dataToUse.end());

  properties.pulseHeight = polarity * (*extremeElement - properties.baseline);
}

void Waveform::calculateIntegrals(int shortGate, int longGate, int polarity) {
  const std::vector<double> &dataToUse = isFiltered() ? filteredData : rawData;

  if (properties.trigger < 0) {
    properties.shortIntegral = 0.0;
    properties.longIntegral = 0.0;
    return;
  }

  auto start = dataToUse.begin() + properties.trigger;
  auto shortEnd = start + shortGate;
  auto longEnd = start + longGate;

  properties.shortIntegral =
      polarity *
      std::accumulate(start, shortEnd, 0.0, [this](double sum, double sample) {
        return sum + (sample - properties.baseline);
      });

  properties.longIntegral =
      polarity *
      std::accumulate(start, longEnd, 0.0, [this](double sum, double sample) {
        return sum + (sample - properties.baseline);
      });
  properties.PSP = (properties.longIntegral - properties.shortIntegral) /
                   properties.longIntegral;
}

void Waveform::analyze(const AnalysisConfig &config) {
  const auto &channelConfig = config.getChannelConfig(channel);

  if (config.isFilterEnabled()) {
    applyFilter(config.getFilterType(), config.getFilterWindowSize());
  }

  calculateBaseline(config.getBaselineSamples());
  findTrigger(channelConfig.threshold, channelConfig.polarity);
  calculatePulseHeight(channelConfig.polarity);
  calculateIntegrals(channelConfig.shortGate, channelConfig.gate,
                     channelConfig.polarity);
}
