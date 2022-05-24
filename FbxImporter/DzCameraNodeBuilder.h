#pragma once

#include "AbstractNodeBuilder.h"
#include "DzBasicCamera.h"

class DzCameraNodeBuilder : public AbstractNodeBuilder {
 public:
  DzCameraNodeBuilder(FbxNode* root);
  ~DzCameraNodeBuilder();

  void buildNode();
};
