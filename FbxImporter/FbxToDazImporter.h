#pragma once

#include "AbstractNodeBuilder.h"
#include "DzBoneNodeBuilder.h"
#include "DzLightNodeBuilder.h"
#include "DzCameraNodeBuilder.h"
#include "DzMeshNodeBuilder.h"
#include "DzNoOpBuilder.h"
#include "DzNullNodeBuilder.h"
#include "LongSummary.h"
#include "MTLParser.h"
#include "RiggingHandler.h"
#include "ShortSummary.h"
#include "VerboseSummary.h"
#include "dzaction.h"
#include "dzprogress.h"
#include "dzscene.h"

class FbxToDazImporter {
 public:
  FbxToDazImporter();
  ~FbxToDazImporter();

  void convert(const QString& fbxFilePath, const QDir& fbxTextureFolderPath,
               const QDir& summaryFolderPath, const QString& mtlFilePath);

  void setTextureFolder(const QString& fbxFilePath,
                        const QDir& fbxTextureFolderPath);

 private:
  QDir mFbxTextureFolderPath;
  FbxManager* mFbxManager;
  FbxScene* mFbxScene;
  AbstractNodeBuilder* mNodeBuilder;
  QMap<FbxNode*, DzNode*> mFbxNodeToDzBone;
  QMap<FbxMesh*, DzFigure*> mFbxMeshToDzFigure;

  void dfs(FbxNode* root, DzNode* prevDzNode,
           const QMap<QString, MTLMap>& materialNameToMTLMap);
  bool importFbxScene(const QString& fbxFileName);
  void createAndDisplaySummary(const QString& fbxFilePath,
                               const QDir& summaryFolderPath);
};
