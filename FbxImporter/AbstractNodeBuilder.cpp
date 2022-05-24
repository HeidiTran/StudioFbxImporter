#include "AbstractNodeBuilder.h"

FbxDouble3 AbstractNodeBuilder::addFbxDouble3(FbxDouble3 a, FbxDouble3 b) {
  return FbxDouble3(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}

void AbstractNodeBuilder::resetNode() { mDzNode = nullptr; }

void AbstractNodeBuilder::setGeometricTransformation(DzGeometry* geom) {
  FbxVector4 geomTranslation =
      mRoot->GetGeometricTranslation(FbxNode::eSourcePivot);
  FbxVector4 rot = mRoot->GetGeometricRotation(FbxNode::eSourcePivot);
  FbxVector4 scaling = mRoot->GetGeometricScaling(FbxNode::eSourcePivot);

  geom->translate(
      DzVec3(geomTranslation[0], geomTranslation[1], geomTranslation[2]));
  geom->rotate(DzQuat(rot[0], rot[1], rot[2], rot[3]));
  geom->scale(DzVec3(scaling[0], scaling[1], scaling[2]));
}

void AbstractNodeBuilder::setNodeBoundingBox(DzNode* node) {
  FbxVector4 bboxMin, bboxMax, bboxCenter;
  mRoot->EvaluateGlobalBoundingBoxMinMaxCenter(bboxMin, bboxMax, bboxCenter);

  DzBox3 boxLocal = node->getLocalBoundingBox();
  boxLocal.setMin(DzVec3(bboxMin[0], bboxMin[1], bboxMin[2]));
  boxLocal.setMax(DzVec3(bboxMax[0], bboxMax[1], bboxMax[2]));
}

void AbstractNodeBuilder::setNodeInheritType(DzNode* node) {
  FbxTransform::EInheritType inheritType;
  mRoot->GetTransformationInheritType(inheritType);

  if (inheritType == FbxTransform::eInheritRrs) {
    node->setInheritScale(false);
  } else {
    node->setInheritScale(true);
  }
}

void AbstractNodeBuilder::setNodePresentation(DzNode* node, QString nodeType) {
  // Get the presentation for the node
  DzPresentation* presentation = node->getPresentation();

  // If the node did not have a presentation,
  // create one and assign it to the node
  if (!presentation) {
    presentation = new DzPresentation();
    node->setPresentation(presentation);
  }

  presentation->setType(nodeType);
}

void AbstractNodeBuilder::setCameraCameraRotation(DzNode* node) {
  FbxDouble3 preRotation = mRoot->PreRotation.Get();
  FbxDouble3 lclRotation = mRoot->LclRotation.Get();

  // Get application name
  FbxDocumentInfo* docInfo = mRoot->GetRootDocument()->GetDocumentInfo();
  FbxString appName = docInfo->Original_ApplicationName;

  FbxString blenderName = "Blender";
  FbxString dazName = "DAZ";

  // If created in blender, apply correction
  if (appName.Left(blenderName.GetLen()) == blenderName ||
      appName.Left(dazName.GetLen()) == dazName) {
    DzVec3 rot = DzVec3((lclRotation[0] * DZ_DBL_DEG_TO_RAD) +
                            (preRotation[0] * DZ_DBL_DEG_TO_RAD),
                        (lclRotation[1] * DZ_DBL_DEG_TO_RAD) +
                            (preRotation[1] * DZ_DBL_DEG_TO_RAD),
                        (lclRotation[2] * DZ_DBL_DEG_TO_RAD) +
                            (preRotation[2] * DZ_DBL_DEG_TO_RAD));

    DzVec3 correction = DzVec3(0, -1.57, 0);

    DzQuat quatRot = DzQuat(node->getRotationOrder(), rot);
    DzQuat quatCorrection = DzQuat(node->getRotationOrder(), correction);
    DzQuat finalRot = quatCorrection * quatRot;

    finalRot.getValue(node->getRotationOrder(), rot);

    node->getXRotControl()->setValue(rot[0] * DZ_DBL_RAD_TO_DEG);
    node->getYRotControl()->setValue(rot[1] * DZ_DBL_RAD_TO_DEG);
    node->getZRotControl()->setValue(rot[2] * DZ_DBL_RAD_TO_DEG);
  } else {
    // Final rotation = PreRotation + LclRotation
    FbxDouble3 rot = addFbxDouble3(preRotation, lclRotation);

    node->getXRotControl()->setValue(rot[0]);
    node->getYRotControl()->setValue(rot[1]);
    node->getZRotControl()->setValue(rot[2]);
  }
}

void AbstractNodeBuilder::setNodeRotation(DzNode* node) {
  FbxDouble3 preRotation = mRoot->PreRotation.Get();
  FbxDouble3 lclRotation = mRoot->LclRotation.Get();

  // Final rotation = PreRotation + LclRotation
  FbxDouble3 rot = addFbxDouble3(preRotation, lclRotation);

  node->getXRotControl()->setValue(rot[0]);
  node->getYRotControl()->setValue(rot[1]);
  node->getZRotControl()->setValue(rot[2]);
}

void AbstractNodeBuilder::setNodeRotationLimits(DzNode* node) {
  FbxLimits rotationLimits = mRoot->GetRotationLimits();
  if (!rotationLimits.GetActive()) {
    return;
  }

  FbxDouble3 min = rotationLimits.GetMin();
  FbxDouble3 max = rotationLimits.GetMax();

  if (rotationLimits.GetMaxXActive() && rotationLimits.GetMinXActive()) {
    node->getXRotControl()->setIsClamped(true);
    node->getXRotControl()->setMinMax(min[0], max[0]);
  }

  if (rotationLimits.GetMaxYActive() && rotationLimits.GetMinYActive()) {
    node->getYRotControl()->setIsClamped(true);
    node->getYRotControl()->setMinMax(min[1], max[1]);
  }

  if (rotationLimits.GetMaxZActive() && rotationLimits.GetMinZActive()) {
    node->getZRotControl()->setIsClamped(true);
    node->getZRotControl()->setMinMax(min[2], max[2]);
  }
}

void AbstractNodeBuilder::setNodeRotationOrder(DzNode* node) {
  FbxEuler::EOrder rotationOrder = mRoot->RotationOrder.Get();
  DzRotationOrder dzRotationOrder(DzRotationOrder::XYZ);
  switch (rotationOrder) {
    case eEulerXYZ:
      dzRotationOrder = DzRotationOrder::XYZ;
      break;
    case eEulerXZY:
      dzRotationOrder = DzRotationOrder::XZY;
      break;
    case eEulerYXZ:
      dzRotationOrder = DzRotationOrder::YXZ;
      break;
    case eEulerYZX:
      dzRotationOrder = DzRotationOrder::YZX;
      break;
    case eEulerZXY:
      dzRotationOrder = DzRotationOrder::ZXY;
      break;
    case eEulerZYX:
      dzRotationOrder = DzRotationOrder::ZYX;
      break;
    default:
      break;
  }
  node->setRotationOrder(dzRotationOrder);
}

void AbstractNodeBuilder::setNodeScaling(DzNode* node) {
  FbxDouble3 scaling = mRoot->LclScaling.Get();
  node->getXScaleControl()->setValue(scaling[0]);
  node->getYScaleControl()->setValue(scaling[1]);
  node->getZScaleControl()->setValue(scaling[2]);
}

void AbstractNodeBuilder::setNodeScalingLimits(DzNode* node) {
  FbxLimits scalingLimits = mRoot->GetScalingLimits();
  if (!scalingLimits.GetActive()) {
    return;
  }

  FbxDouble3 min = scalingLimits.GetMin();
  FbxDouble3 max = scalingLimits.GetMax();

  if (scalingLimits.GetMaxXActive() && scalingLimits.GetMinXActive()) {
    node->getXScaleControl()->setIsClamped(true);
    node->getXScaleControl()->setMinMax(min[0], max[0]);
  }

  if (scalingLimits.GetMaxYActive() && scalingLimits.GetMinYActive()) {
    node->getYScaleControl()->setIsClamped(true);
    node->getYScaleControl()->setMinMax(min[1], max[1]);
  }

  if (scalingLimits.GetMaxZActive() && scalingLimits.GetMinZActive()) {
    node->getZScaleControl()->setIsClamped(true);
    node->getZScaleControl()->setMinMax(min[2], max[2]);
  }
}

void AbstractNodeBuilder::setNodeTranslation(DzNode* node) {
  FbxDouble3 translation = mRoot->LclTranslation.Get();
  node->getXPosControl()->setValue(translation[0]);
  node->getYPosControl()->setValue(translation[1]);
  node->getZPosControl()->setValue(translation[2]);
}

void AbstractNodeBuilder::setNodeTranslationLimits(DzNode* node) {
  FbxLimits translationLimits = mRoot->GetTranslationLimits();
  if (!translationLimits.GetActive()) {
    return;
  }

  FbxDouble3 min = translationLimits.GetMin();
  FbxDouble3 max = translationLimits.GetMax();

  if (translationLimits.GetMaxXActive() && translationLimits.GetMinXActive()) {
    node->getXPosControl()->setIsClamped(true);
    node->getXPosControl()->setMinMax(min[0], max[0]);
  }

  if (translationLimits.GetMaxYActive() && translationLimits.GetMinYActive()) {
    node->getYPosControl()->setIsClamped(true);
    node->getYPosControl()->setMinMax(min[1], max[1]);
  }

  if (translationLimits.GetMaxZActive() && translationLimits.GetMinZActive()) {
    node->getZPosControl()->setIsClamped(true);
    node->getZPosControl()->setMinMax(min[2], max[2]);
  }
}

void AbstractNodeBuilder::setNodeVisibility(DzNode* node) {
  node->getVisibilityControl()->setBoolValue(mRoot->GetVisibility());
}

void AbstractNodeBuilder::setNodeOrientation(DzNode* node) {
  FbxVector4 fbxPre = mRoot->GetPreRotation(FbxNode::eSourcePivot);
  const DzQuat rot(DzRotationOrder::XYZ,
                   DzVec3(fbxPre[0], fbxPre[1], fbxPre[2]) * DZ_FLT_DEG_TO_RAD);

  node->setOrientation(rot, true);
}

void AbstractNodeBuilder::setNodeProperties(DzNode* node) {
  setNodeRotationOrder(node);
  setNodeRotation(node);
  setNodeRotationLimits(node);
  setNodeTranslation(node);
  setNodeTranslationLimits(node);
  setNodeScaling(node);
  setNodeScalingLimits(node);
  setNodeVisibility(node);
  setNodeOrientation(node);
  setNodeInheritType(node);
}

void AbstractNodeBuilder::setCameraNodeProperties(DzNode* node) {
  setNodeRotationOrder(node);
  setCameraCameraRotation(node);
  setNodeRotationLimits(node);
  setNodeTranslation(node);
  setNodeTranslationLimits(node);
  setNodeVisibility(node);
  setNodeOrientation(node);
  setNodeInheritType(node);
}

DzNode* AbstractNodeBuilder::getNode() {
  DzNode* result = mDzNode;
  resetNode();
  return result;
}
