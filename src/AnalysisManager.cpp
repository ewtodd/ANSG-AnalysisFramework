#include "AnalysisManager.h"

namespace fs = std::filesystem;

AnalysisManager::AnalysisManager(const std::string &path, int baselineSamples)
    : basePath(path) {
  std::string xmlPath = getXMLPath();
  config = std::make_unique<AnalysisConfig>(xmlPath, baselineSamples);
}

std::string AnalysisManager::getXMLPath() const {
  return fs::path(basePath) / "settings.xml";
}

std::vector<std::string> AnalysisManager::findDataFiles() const {
  std::vector<std::string> dataFiles;
  fs::path rawPath = fs::path(basePath) / "RAW";

  if (!fs::exists(rawPath)) {
    throw std::runtime_error("RAW directory not found in " + basePath);
  }

  std::regex filePattern("DataR_CH\\d+@DT5730B.*\\.root$");
  for (const auto &entry : fs::directory_iterator(rawPath)) {
    if (std::regex_match(entry.path().filename().string(), filePattern)) {
      dataFiles.push_back(entry.path().string());
    }
  }
  return dataFiles;
}

void AnalysisManager::processChannel(const std::string &rootFile, int channel) {
  TFile file(rootFile.c_str(), "READ");
  if (file.IsZombie()) {
    throw std::runtime_error("Could not open file: " + rootFile);
  }

  TTree *tree = (TTree *)file.Get("DataR");
  if (!tree) {
    throw std::runtime_error("Could not find DataR tree in file: " + rootFile);
  }

  std::vector<uint16_t> *samples = nullptr;
  tree->SetBranchAddress("Samples", &samples);

  double samplingRate = 0.5; // ns, assuming 2GS/s

  for (Long64_t i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);

    // Convert uint16_t samples to double
    std::vector<double> doubleSamples(samples->begin(), samples->end());

    auto waveform =
        std::make_unique<Waveform>(doubleSamples, samplingRate, channel);
    waveform->analyze(*config);
    waveforms.push_back(std::move(waveform));
  }
}

void AnalysisManager::processData() {
  auto dataFiles = findDataFiles();
  auto enabledChannels = config->getEnabledChannels();

  for (const auto &file : dataFiles) {
    // Extract channel number from filename
    std::smatch match;
    std::regex channelPattern("DataR_CH(\\d+)@");
    std::string filename = fs::path(file).filename().string();

    if (std::regex_search(filename, match, channelPattern)) {
      int channel = std::stoi(match[1]);

      // Only process enabled channels
      if (std::find(enabledChannels.begin(), enabledChannels.end(), channel) !=
          enabledChannels.end()) {
        processChannel(file, channel);
      }
    }
  }
}
