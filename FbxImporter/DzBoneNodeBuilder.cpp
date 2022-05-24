#include "DzBoneNodeBuilder.h"

DzBoneNodeBuilder::DzBoneNodeBuilder(
    FbxNode* root, QMap<FbxNode*, DzNode*>* fbxNodeToDzBonePtr) {
  mRoot = root;
  mFbxNodeToDzBonePtr = fbxNodeToDzBonePtr;
  mFbxSceneRootNode = mRoot->GetScene()->GetRootNode();
}

DzBoneNodeBuilder::~DzBoneNodeBuilder() { delete mDzNode; }

void DzBoneNodeBuilder::buildNode() {
  FbxSkeleton* fbxSkeleton = (FbxSkeleton*)mRoot->GetNodeAttribute();

  if (fbxSkeleton->GetSkeletonType() == FbxSkeleton::eRoot) {
    mDzNode = DzHelpers::createFigure();
    mDzNode->setName(mRoot->GetName());
    mDzNode->setLabel(mDzNode->getName());
    mDzNode->setInheritScale(false);
    setNodeVisibility(mDzNode);
    setNodeRotationOrder(mDzNode);
  } else if (fbxSkeleton->GetSkeletonType() == FbxSkeleton::eLimb ||
             fbxSkeleton->GetSkeletonType() == FbxSkeleton::eLimbNode) {
    if (mRoot->GetParent() == mFbxSceneRootNode) {
      mDzNode = DzHelpers::createFigure();
    } else {
      mDzNode = new DzBone();
      mFbxNodeToDzBonePtr->insert(mRoot, mDzNode);
    }

    mDzNode->setName(mRoot->GetName());
    mDzNode->setLabel(mDzNode->getName());
    mDzNode->setInheritScale(false);
    setNodeVisibility(mDzNode);
    setNodeRotationOrder(mDzNode);
  } else {
    DzHelpers::logInfo("Found an eEffector!");
  }
}
