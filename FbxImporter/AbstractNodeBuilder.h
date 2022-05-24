#pragma once

#include "DzHelpers.h"
#include "SummarySingleton.h"
#include "dzboolproperty.h"
#include "dzdefaultmaterial.h"
#include "dzfacetmesh.h"
#include "dzfacetshape.h"
#include "dzfloatproperty.h"
#include "dznode.h"
#include "dzobject.h"
#include "dzpresentation.h"
#include "fbxsdk.h"

class AbstractNodeBuilder {
 public:
  virtual ~AbstractNodeBuilder(){};

  virtual void buildNode() = 0;
  DzNode* getNode();

 protected:
  DzNode* mDzNode;
  FbxNode* mRoot;

  FbxDouble3 addFbxDouble3(FbxDouble3 a, FbxDouble3 b);
  void resetNode();
  void setGeometricTransformation(DzGeometry* geom);
  void setNodeBoundingBox(DzNode* node);
  void setNodeOrientation(DzNode* node);
  void setNodeInheritType(DzNode* node);
  void setNodePresentation(DzNode* node, QString nodeType);
  void setCameraCameraRotation(DzNode* node);
  void setNodeRotation(DzNode* node);
  void setNodeRotationLimits(DzNode* node);
  void setNodeRotationOrder(DzNode* node);
  void setNodeScaling(DzNode* node);
  void setNodeScalingLimits(DzNode* node);
  void setNodeTranslation(DzNode* node);
  void setNodeTranslationLimits(DzNode* node);
  void setNodeVisibility(DzNode* node);
  void setNodeProperties(DzNode* node);
  void setCameraNodeProperties(DzNode* node);
};
