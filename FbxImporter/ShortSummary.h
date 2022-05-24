#pragma once

#include "Summary.h"

class ShortSummary : public Summary {
 public:
  ShortSummary(QString longSummaryPath);
  ~ShortSummary();

  void createUnknownMaterialsSection();
  void createMissingTextureFilesSection();
  void createReplacedTextureFilesSection();
  void createUnsupportedTexturesSection();
  void footerHook();

 private:
  const int DISPLAY_LIMIT = 3;
  QString mLongSummaryPath;
};
