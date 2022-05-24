#pragma once

#include "DzHelpers.h"
#include "dzaction.h"
#include "dzactionmgr.h"
#include "dzbone.h"
#include "dzbonebinding.h"
#include "dzfigure.h"
#include "dzscene.h"
#include "dzskinbinding.h"
#include "dzweightmap.h"
#include "fbxsdk.h"

class RiggingHandler {
 public:
  RiggingHandler(FbxScene* fbxScene, DzScene* dzScene, DzActionMgr* actionMgr,
                 const QMap<FbxNode*, DzNode*>& fbxNodeToDzBone,
                 const QMap<FbxMesh*, DzFigure*>& fbxMeshToDzFigure);
  ~RiggingHandler();

  void handle();

 private:
  struct FbxWeightToDzWeight {
    QVector<double> fbxVertexWeights;
    unsigned short* dzVertexWeights;
  };

  DzActionMgr* mActionMgr;
  FbxScene* mFbxScene;
  DzScene* mDzScene;
  QMap<FbxNode*, DzNode*> mFbxNodeToDzBone;
  QMap<FbxMesh*, DzFigure*> mFbxMeshToDzFigure;
  DzAction* mJEAlignAction;

  void handleBones();
  void setOriginAndEndPoint(const fbxsdk::FbxPose* bindPose);
  void handleSkinBinding();
};
