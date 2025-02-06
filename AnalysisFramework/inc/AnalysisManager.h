#ifndef ANALYSISMANAGER_H
#define ANALYSISMANAGER_H

#include "AnalysisConfig.h"
#include "Waveform.h"
#include <TFile.h>
#include <TTree.h>
#include <filesystem>
#include <memory>
#include <regex>
#include <string>
#include <vector>

class AnalysisManager {
public:
  AnalysisManager(const std::string &basePath, int baselineSamples);
  void processData();
  void saveResults(const std::string &outputPath);

private:
  std::unique_ptr<AnalysisConfig> config;
  std::string basePath;
  std::vector<std::unique_ptr<Waveform>> waveforms;

  std::string getXMLPath() const;
  std::vector<std::string> findDataFiles() const;
  void processChannel(const std::string &rootFile, int channel);
};

#endif
