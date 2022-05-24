#pragma once

#include "QtGui"
#include "SummarySingleton.h"

// Summary is structured using the Template Method design pattern
class Summary {
 public:
  Summary();
  ~Summary();

  // This is the template method, which is final so that subclasses
  // cannot override this method
  virtual QString create() final;

 protected:
  QString mSummary;
  // These functions are defined in Summary and shared by subclasses
  virtual void createHeader() final;
  virtual void createNormalMappedByVertexSection() final;
  virtual void createNotSupportedSection() final;

  // These functions may or may not be implemented in subclasses, depending
  // on whether or not they wish to override the default implementation
  virtual void createUnknownMaterialsSection();
  virtual void createMissingTextureFilesSection();
  virtual void createReplacedTextureFilesSection();
  virtual void createUnsupportedTexturesSection();

  // These are "hooks," which mean subclasses may override them, but
  // it is not strictly necessary because hooks already have a default
  // (but empty) implementation
  virtual void sectionHook();
  virtual void footerHook();
};
