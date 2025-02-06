#ifndef ANALYSISCONFIG_H
#define ANALYSISCONFIG_H

#include "FilterType.h"
#include <map>
#include <pugixml.hpp>
#include <string>
#include <vector>

class AnalysisConfig {
public:
  struct ChannelConfig {
    int gate;
    int shortGate;
    int preGate;
    int recordLength;
    int threshold;
    std::string triggerMode;
    double energyGain;
    int polarity;
  };

  struct FilterConfig {
    bool enabled;
    int windowSize;
    FilterType type;
  };

  AnalysisConfig(const std::string &xmlFile, int baselineSamples,
                 bool filterEnabled = false,
                 FilterType filterType = FilterType::MovingAverage,
                 int filterWindow = 5);

  bool isFilterEnabled() const { return filterConfig.enabled; }
  int getFilterWindowSize() const { return filterConfig.windowSize; }
  FilterType getFilterType() const { return filterConfig.type; }
  int getBaselineSamples() const { return baselineSamples; }
  const ChannelConfig &getChannelConfig(int channel) const;
  std::vector<int> getEnabledChannels() const;

private:
  FilterConfig filterConfig;
  int baselineSamples;
  std::map<int, ChannelConfig> channelConfigs;

  void parseXMLConfig(const std::string &xmlFile);
  std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
  extractParameterValues(pugi::xml_node &board, const std::string &paramKey);
};
#endif
