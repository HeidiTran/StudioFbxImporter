#pragma once

#include "AbstractNodeBuilder.h"
#include "dzcolorproperty.h"
#include "dzdistantlight.h"
#include "dzlight.h"
#include "dzpointlight.h"
#include "dzspotlight.h"

class DzLightNodeBuilder : public AbstractNodeBuilder {
 public:
  DzLightNodeBuilder(FbxNode* root);
  ~DzLightNodeBuilder();

  void buildNode();
  FbxString getLightTypeName(FbxLight::EType type);

 protected:
  void setColorIntensity(DzDistantLight* light, FbxDouble intensity,
                         FbxDouble3 color);
};
