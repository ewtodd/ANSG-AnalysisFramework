#include "AnalysisConfig.h"

AnalysisConfig::AnalysisConfig(const std::string &xmlFile, int baseline,
                               bool filterEnabled, FilterType filterType,
                               int filterWindow)
    : baselineSamples(baseline) {
  filterConfig.enabled = filterEnabled;
  filterConfig.type = filterType;
  filterConfig.windowSize = filterWindow;
  parseXMLConfig(xmlFile);
}

void AnalysisConfig::parseXMLConfig(const std::string &xmlFile) {
  pugi::xml_document doc;
  if (!doc.load_file(xmlFile.c_str())) {
    throw std::runtime_error("Failed to load XML file");
  }

  pugi::xml_node root = doc.child("board");
  if (!root) {
    throw std::runtime_error("Invalid XML structure: no board found");
  }

  // Get enabled channels first
  auto [_, enabledChannels] =
      extractParameterValues(root, "SRV_PARAM_CH_ENABLED");

  for (const auto &[channel, enabled] : enabledChannels) {
    if (enabled != "true")
      continue;

    int channelNum = std::stoi(channel);
    ChannelConfig config;

    // Extract all parameters for this channel
    auto [_, gateValues] = extractParameterValues(root, "SRV_PARAM_CH_GATE");
    auto [__, shortGateValues] =
        extractParameterValues(root, "SRV_PARAM_CH_GATESHORT");
    auto [___, preGateValues] =
        extractParameterValues(root, "SRV_PARAM_CH_GATEPRE");
    auto [____, polarityValues] =
        extractParameterValues(root, "SRV_PARAM_CH_POLARITY");

    // Find channel-specific values
    for (const auto &[ch, value] : gateValues) {
      if (ch == channel) {
        // Keep gates in ns as they are in the XML
        config.gate = std::stoi(value);                          // ns
        config.shortGate = std::stoi(shortGateValues[0].second); // ns
        config.preGate = std::stoi(preGateValues[0].second);     // ns
        config.polarity =
            (polarityValues[0].second == "POLARITY_POSITIVE") ? 1 : -1;
        channelConfigs[channelNum] = config;
        break;
      }
    }
  }
}

std::pair<std::string, std::vector<std::pair<std::string, std::string>>>
AnalysisConfig::extractParameterValues(pugi::xml_node &board,
                                       const std::string &paramKey) {
  std::string globalValue;
  std::vector<std::pair<std::string, std::string>> channelValues;

  // Check global parameters
  for (pugi::xml_node entry : board.child("parameters").children("entry")) {
    if (entry.child("key").text().get() == paramKey) {
      globalValue = entry.child("value").child("value").text().get();
      break;
    }
  }

  // Check channel-specific parameters
  for (pugi::xml_node channel : board.children("channel")) {
    std::string channelIdx = channel.child("index").text().get();
    for (pugi::xml_node entry : channel.child("values").children("entry")) {
      if (entry.child("key").text().get() == paramKey) {
        std::string value = entry.child("value").text().get();
        channelValues.emplace_back(channelIdx, value);
        break;
      }
    }
  }

  return {globalValue, channelValues};
}

const AnalysisConfig::ChannelConfig &
AnalysisConfig::getChannelConfig(int channel) const {
  auto it = channelConfigs.find(channel);
  if (it == channelConfigs.end()) {
    throw std::runtime_error("Channel configuration not found");
  }
  return it->second;
}

std::vector<int> AnalysisConfig::getEnabledChannels() const {
  std::vector<int> channels;
  for (const auto &[channel, _] : channelConfigs) {
    channels.push_back(channel);
  }
  return channels;
}
