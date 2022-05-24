#pragma once

#include "Summary.h"

class VerboseSummary : public Summary {
 public:
  VerboseSummary();
  ~VerboseSummary();

  void createMissingTextureFilesSection();
  void sectionHook();

 private:
  void createMaterialNamesSection();
  void createNodeNameToMaterialsSection();
  void createNodeInfoToTextureFileSection();
};
