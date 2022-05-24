#include "DzCameraNodeBuilder.h"

DzCameraNodeBuilder::DzCameraNodeBuilder(FbxNode* root) { mRoot = root; }

DzCameraNodeBuilder::~DzCameraNodeBuilder() {}

void DzCameraNodeBuilder::buildNode() {
  FbxCamera* fbxCamera = mRoot->GetCamera();
  DzCamera* dzCamera = new DzBasicCamera(DzCamera::GENERAL_CAMERA);

  dzCamera->setFocalLength(fbxCamera->FocalLength.Get());

  mDzNode = dzCamera;
  mDzNode->setName(mRoot->GetName());
  setCameraNodeProperties(mDzNode);
}
