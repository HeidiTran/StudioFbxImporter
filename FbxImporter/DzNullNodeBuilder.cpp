#include "DzNullNodeBuilder.h"

DzNullNodeBuilder::DzNullNodeBuilder(
    FbxNode* root, QMap<FbxNode*, DzNode*>* fbxNodeToDzBonePtr) {
  mRoot = root;
  mFbxNodeToDzBonePtr = fbxNodeToDzBonePtr;
  mFbxSceneRootNode = mRoot->GetScene()->GetRootNode();
}

DzNullNodeBuilder::~DzNullNodeBuilder() { delete mDzNode; }

void DzNullNodeBuilder::buildNode() {
  // Construct the correct type of node
  mDzNode = new DzNode();
  bool isTopLevelNode = mRoot->GetParent() == mFbxSceneRootNode;
  bool isFigure = isTopLevelNode && hasChildWithBoneAttribute();
  if (isFigure) {
    mDzNode = DzHelpers::createFigure();
  }

  mDzNode->setName(mRoot->GetName());
  mDzNode->setLabel(mDzNode->getName());
  setNodeProperties(mDzNode);

  if (isFigure) {
    // Undo setNodeRotation() inside setNodeProperties
    mDzNode->getXRotControl()->setValue(0);
    mDzNode->getYRotControl()->setValue(0);
    mDzNode->getZRotControl()->setValue(0);
  }
}

bool DzNullNodeBuilder::hasAncestorWithBoneAttribute() {
  FbxNode* ancestor = mRoot->GetParent();
  while (ancestor != nullptr) {
    FbxNodeAttribute* ancestorNodeAttribute = ancestor->GetNodeAttribute();
    if (ancestorNodeAttribute != nullptr &&
        ancestorNodeAttribute->GetAttributeType() ==
            FbxNodeAttribute::eSkeleton) {
      return true;
    }

    ancestor = ancestor->GetParent();
  }

  return false;
}

bool DzNullNodeBuilder::hasChildWithBoneAttribute() {
  for (int i = 0; i < mRoot->GetChildCount(false); i++) {
    FbxNodeAttribute* childNodeAttribute =
        mRoot->GetChild(i)->GetNodeAttribute();
    if (childNodeAttribute != nullptr &&
        childNodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
      return true;
    }
  }

  return false;
}
