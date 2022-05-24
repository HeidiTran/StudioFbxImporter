#pragma once

#include "AbstractNodeBuilder.h"
#include "dzbone.h"

class DzNullNodeBuilder : public AbstractNodeBuilder {
 public:
  DzNullNodeBuilder(FbxNode* root, QMap<FbxNode*, DzNode*>* fbxNodeToDzBonePtr);
  ~DzNullNodeBuilder();

  void buildNode();

 private:
  QMap<FbxNode*, DzNode*>* mFbxNodeToDzBonePtr;
  FbxNode* mFbxSceneRootNode;

  bool hasAncestorWithBoneAttribute();
  bool hasChildWithBoneAttribute();
};
