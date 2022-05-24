#pragma once

#include "AbstractNodeBuilder.h"
#include "dzbone.h"
#include "dzskeleton.h"

class DzBoneNodeBuilder : public AbstractNodeBuilder {
 public:
  DzBoneNodeBuilder(FbxNode* root, QMap<FbxNode*, DzNode*>* fbxNodeToDzBonePtr);
  ~DzBoneNodeBuilder();

  void buildNode();

 private:
  QMap<FbxNode*, DzNode*>* mFbxNodeToDzBonePtr;
  FbxNode* mFbxSceneRootNode;
};
