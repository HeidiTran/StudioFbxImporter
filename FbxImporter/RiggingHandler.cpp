#include "RiggingHandler.h"

RiggingHandler::RiggingHandler(
    FbxScene* fbxScene, DzScene* dzScene, DzActionMgr* actionMgr,
    const QMap<FbxNode*, DzNode*>& fbxNodeToDzBone,
    const QMap<FbxMesh*, DzFigure*>& fbxMeshToDzFigure) {
  mActionMgr = actionMgr;
  mFbxNodeToDzBone = fbxNodeToDzBone;
  mFbxMeshToDzFigure = fbxMeshToDzFigure;
  mFbxScene = fbxScene;
  mDzScene = dzScene;
  mJEAlignAction = actionMgr->findAction("DzJEAlignNodeAction");
}

RiggingHandler::~RiggingHandler() {}

void RiggingHandler::handle() { handleBones(); }

void RiggingHandler::handleBones() {
  const FbxPose* bindPose{};
  for (int i = 0; i < mFbxScene->GetPoseCount(); i++) {
    const FbxPose* pose = mFbxScene->GetPose(i);
    if (pose->IsBindPose()) {
      bindPose = pose;
      break;
    }
  }

  if (bindPose) {
    setOriginAndEndPoint(bindPose);
  }
}

void RiggingHandler::setOriginAndEndPoint(const fbxsdk::FbxPose* bindPose) {
  auto iter = mFbxNodeToDzBone.constBegin();
  while (iter != mFbxNodeToDzBone.constEnd()) {
    FbxNode* node = iter.key();
    DzNode* bone = iter.value();

    if (node->GetParent() && node->GetParent() != mFbxScene->GetRootNode()) {
      FbxAMatrix globalPos = DzHelpers::GetGlobalPosition(
          node, FbxTime(), bindPose, new FbxAMatrix());
      FbxVector4 translation = globalPos.GetT();
      bone->setOrigin(DzVec3(translation[0], translation[1], translation[2]));

      if (node->GetChildCount(false) == 1) {
        FbxAMatrix childGlobalPos = DzHelpers::GetGlobalPosition(
            node->GetChild(0), FbxTime(), bindPose, new FbxAMatrix());
        FbxVector4 childTranslation = childGlobalPos.GetT();
        bone->setEndPoint(DzVec3(childTranslation[0], childTranslation[1],
                                 childTranslation[2]));
      } else {
        bone->setEndPoint(
            DzVec3(translation[0], translation[1], translation[2]));
      }

      if (node->GetChildCount(false) > 1) {
        FbxAMatrix parentGlobalPos = DzHelpers::GetGlobalPosition(
            node->GetParent(), FbxTime(), bindPose, new FbxAMatrix());
        FbxVector4 parentTranslation = parentGlobalPos.GetT();
        bone->setOrigin(DzVec3(parentTranslation[0], parentTranslation[1],
                               parentTranslation[2]));
      }
    }

    bone->select(true);
    mJEAlignAction->trigger();
    bone->select(false);

    ++iter;
  }
}

// TODO: This does not work currently 
void RiggingHandler::handleSkinBinding() {
  auto iter = mFbxMeshToDzFigure.constBegin();
  while (iter != mFbxMeshToDzFigure.constEnd()) {
    FbxMesh* fbxMesh = iter.key();
    DzFigure* dzFigure = iter.value();

    // TODO: What if one mesh has multiple skins?
    // Will dzFigure's skin binding be overriden?
    DzSkinBinding* dzSkinBinding = dzFigure->getSkinBinding();
    if (!dzSkinBinding) {
      DzHelpers::logInfo("Skin Binding not found!");
      ++iter;
      continue;
    }

    int skinCnt = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
    DzHelpers::logInfo("Skin count: " + QString::number(skinCnt));
    for (int i = 0; i < skinCnt; i++) {
      FbxSkin* fbxSkin =
          ((FbxSkin*)fbxMesh->GetDeformer(i, FbxDeformer::eSkin));

      int meshVerticesCnt = fbxMesh->GetControlPointsCount();
      int targetVertexCount = 0;
      if (QMetaObject::invokeMethod(dzSkinBinding, "getTargetVertexCount",
                                    Q_RETURN_ARG(int, targetVertexCount)) &&
          targetVertexCount < 1) {
        QMetaObject::invokeMethod(
            dzSkinBinding, "setTargetVertexCount",
            Q_ARG(int, fbxSkin->GetControlPointIndicesCount()));
      }

      DzWeightMapList maps;
      QVector<FbxWeightToDzWeight> fbxWeightToDzWeights;
      int clusterCnt = fbxSkin->GetClusterCount();
      DzHelpers::logInfo("Cluster count: " + QString::number(clusterCnt));
      for (int j = 0; j < clusterCnt; j++) {
        FbxCluster* fbxCluster = fbxSkin->GetCluster(j);

        if (fbxCluster->GetLink() == NULL) {
          DzHelpers::logWarning("FbxCluster doesn't have any link!");
          continue;
        }

        if (!mFbxNodeToDzBone.contains(fbxCluster->GetLink())) {
          DzHelpers::logWarning("Missing bones!");
          DzHelpers::logInfo(fbxCluster->GetLink()->GetName());
          continue;
        }

        DzBone* dzBone = (DzBone*)(mFbxNodeToDzBone[fbxCluster->GetLink()]);
        if (!dzBone) {
          DzHelpers::logWarning("Casting to DzBone failed!");
          continue;
        }

        DzBoneBinding* dzBoneBinding = new DzBoneBinding();
        dzBoneBinding->setBone(dzBone);
        dzSkinBinding->addBoneBinding(dzBoneBinding);

        int vertexIndicesCnt = fbxCluster->GetControlPointIndicesCount();
        int* fbxVertexIndices = fbxCluster->GetControlPointIndices();
        double* fbxVertexWeights = fbxCluster->GetControlPointWeights();

        DzWeightMap* dzWeightMap = new DzWeightMap(meshVerticesCnt);
        unsigned short* dzWeights = dzWeightMap->getWeights();

        FbxWeightToDzWeight fbxWeightToDzWeight;
        fbxWeightToDzWeight.dzVertexWeights = dzWeightMap->getWeights();
        fbxWeightToDzWeight.fbxVertexWeights.resize(meshVerticesCnt);
        for (int k = 0; k < vertexIndicesCnt; k++) {
          fbxWeightToDzWeight.fbxVertexWeights[fbxVertexIndices[k]] =
              fbxVertexWeights[k];
          dzWeights[fbxVertexIndices[k]] =
              static_cast<unsigned short>(fbxVertexWeights[k]);
        }
        fbxWeightToDzWeights.append(fbxWeightToDzWeight);
        dzBoneBinding->setWeights(dzWeightMap);
        maps.append(dzWeightMap);

        // TODO: Get transformation matrix here
        DzMatrix3 dzMatrix;
        FbxAMatrix fbxMatrix;
        fbxCluster->GetTransformLinkMatrix(fbxMatrix);
        for (int m = 0; m < 3; m++) {
          for (int n = 0; n < 3; n++) {
            dzMatrix[m][n] = fbxMatrix[m][n];
          }
        }

        // TODO: Why - sign?
        dzMatrix[3][0] = -fbxMatrix[3][0];
        dzMatrix[3][1] = -fbxMatrix[3][1];
        dzMatrix[3][2] = -fbxMatrix[3][2];

        // TODO: What does this do?
        DzVec3 skelOrigin = dzFigure->getOrigin();
        DzVec3 origin = dzBone->getOrigin();
        dzMatrix[3][0] += (origin[0] - skelOrigin[0]);
        dzMatrix[3][1] += (origin[1] - skelOrigin[1]);
        dzMatrix[3][2] += (origin[2] - skelOrigin[2]);

        dzBoneBinding->setBindingMatrix(dzMatrix);
      }

      // Convert FBX's vertex weights To Daz's vertex weights
      for (int j = 0; j < meshVerticesCnt; j++) {
        double sum = 0.0;
        for (int k = 0; k < fbxWeightToDzWeights.count(); k++) {
          sum += fbxWeightToDzWeights[k].fbxVertexWeights[j];
        }

        for (int k = 0; k < fbxWeightToDzWeights.count(); k++) {
          fbxWeightToDzWeights[k].dzVertexWeights[j] =
              static_cast<unsigned short>(
                  (fbxWeightToDzWeights[k].fbxVertexWeights[j] / sum) *
                  DZ_USHORT_MAX);
        }
      }

      DzWeightMap::normalizeMaps(maps);
      QMetaObject::invokeMethod(dzSkinBinding, "setBindingMode", Q_ARG(int, 0));
      QMetaObject::invokeMethod(dzSkinBinding, "setScaleMode", Q_ARG(int, 1));
    }

    ++iter;
  }
}
