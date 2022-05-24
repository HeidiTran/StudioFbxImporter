#include "FbxToDazImporter.h"

FbxToDazImporter::FbxToDazImporter() {
  mFbxManager = FbxManager::Create();
  FbxIOSettings* ioSettings = FbxIOSettings::Create(mFbxManager, IOSROOT);
  mFbxManager->SetIOSettings(ioSettings);

  mFbxScene = nullptr;
  mNodeBuilder = nullptr;
  mFbxNodeToDzBone = {};
  mFbxMeshToDzFigure = {};
}

FbxToDazImporter::~FbxToDazImporter() {
  mFbxManager->Destroy();
  mFbxNodeToDzBone.clear();
  mFbxMeshToDzFigure.clear();
}

void FbxToDazImporter::convert(const QString& fbxFilePath,
                               const QDir& fbxTextureFolder,
                               const QDir& summaryFolder,
                               const QString& mtlFilePath) {
  MTLParser parser;
  QMap<QString, MTLMap> materialNameToMTLMap = parser.parse(mtlFilePath);

  // Create an FBX scene object to hold the scene that was imported
  mFbxScene = FbxScene::Create(mFbxManager, "");
  bool ok = importFbxScene(fbxFilePath);
  if (!ok) {
    DzHelpers::logError("FBX SDK failed to import the scene!");
    return;
  }

  // IMPORTANT: Has to be call AFTER importFbxScene
  setTextureFolder(fbxFilePath, fbxTextureFolder);

  FbxNode* sceneRootNode = mFbxScene->GetRootNode();
  int sceneImmediateChildCnt = sceneRootNode->GetChildCount(false);

  // +2 for importFbxScene step and triangulation step
  DzProgress importProgress("Importing " + fbxFilePath,
                            sceneImmediateChildCnt + 2, false, true);
  importProgress.setShowTimeEstimate(true);
  importProgress.step();

  // Create a geometry converter to triangulate all the polygons in the
  // FBX scene since Daz only accepts triangles and quadrilaterals (Performance bottle neck here!)
  FbxGeometryConverter geoConverter(mFbxManager);
  geoConverter.Triangulate(mFbxScene, true);
  importProgress.step();

  for (int i = 0; i < sceneImmediateChildCnt; i++) {
    try {
      dfs(sceneRootNode->GetChild(i), nullptr, materialNameToMTLMap);
    } catch (const std::logic_error& e) {
      DzHelpers::logAndDisplayCritical("FbxImporter Error", e.what());
      break;
    }

    importProgress.step();
  }

  RiggingHandler* riggingHandler = new RiggingHandler(
      mFbxScene, dzScene, dzApp->getInterface()->getActionMgr(),
      mFbxNodeToDzBone, mFbxMeshToDzFigure);
  riggingHandler->handle();

  createAndDisplaySummary(fbxFilePath, summaryFolder);
  importProgress.finish();

  mFbxScene->Destroy(true);
}

/// <summary>
/// When a binary FBX file containing embedded media is imported, the media will
/// be extracted from the file and copied into a subdirectory. By default, this
/// subdirectory will be created at the location of the FBX file with the same
/// name as the file, with the extension .fbm
/// Reference:
/// https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_importing_and_exporting_a_scene_referencing_media_html
/// </summary>
/// <param name="fbxFilePath"></param>
void FbxToDazImporter::setTextureFolder(const QString& fbxFilePath,
                                        const QDir& fbxTextureFolder) {
  QDir embeddedMediaFolder =
      QDir(fbxFilePath.left(fbxFilePath.length() - 1) + "m");
  if (embeddedMediaFolder.exists()) {
    mFbxTextureFolderPath = embeddedMediaFolder;
  } else {
    mFbxTextureFolderPath = fbxTextureFolder;
  }
}

void FbxToDazImporter::dfs(FbxNode* root, DzNode* prevDzNode,
                           const QMap<QString, MTLMap>& materialNameToMTLMap) {
  DzNode* curDzNode = nullptr;

  FbxNodeAttribute* nodeAttribute = root->GetNodeAttribute();
  if (!nodeAttribute) {
    mNodeBuilder = new DzNullNodeBuilder(root, &mFbxNodeToDzBone);
  } else {
    switch (nodeAttribute->GetAttributeType()) {
      case FbxNodeAttribute::eNull:
        mNodeBuilder = new DzNullNodeBuilder(root, &mFbxNodeToDzBone);
        break;
      case FbxNodeAttribute::eMesh:
        mNodeBuilder =
            new DzMeshNodeBuilder(root, mFbxTextureFolderPath,
                                  materialNameToMTLMap, &mFbxMeshToDzFigure);
        break;
      case FbxNodeAttribute::eCamera:
        mNodeBuilder = new DzCameraNodeBuilder(root);
        break;
      case FbxNodeAttribute::eSkeleton:
        mNodeBuilder = new DzBoneNodeBuilder(root, &mFbxNodeToDzBone);
        break;
      case FbxNodeAttribute::eLight:
        mNodeBuilder = new DzLightNodeBuilder(root);
        break;
      default:
        mNodeBuilder = new DzNoOpBuilder();
        break;
    }
  }

  mNodeBuilder->buildNode();
  curDzNode = mNodeBuilder->getNode();

  if (curDzNode) {
    dzScene->addNode(curDzNode);

    bool isChildNode =
        prevDzNode && !FbxRootNodeUtility::IsFbxRootNode(root->GetParent());
    if (isChildNode) {
      prevDzNode->addNodeChild(curDzNode);
    }
  }

  for (int i = 0; i < root->GetChildCount(false); i++) {
    try {
      dfs(root->GetChild(i), curDzNode, materialNameToMTLMap);
    } catch (const std::logic_error& e) {
      throw e;
    }
  }
}

bool FbxToDazImporter::importFbxScene(const QString& fbxFilePath) {
  // TODO: We're not importing animation info right now for import speed reason
  FbxIOSettings* importSettings = mFbxManager->GetIOSettings();
  importSettings->SetBoolProp(IMP_FBX_ANIMATION, true);

  // Import the desired FBX file and check that the import was successful
  FbxImporter* importer = FbxImporter::Create(mFbxManager, "");

  if (!importer->Initialize(fbxFilePath.toLatin1().data(), -1,
                            importSettings)) {
    DzHelpers::displayCriticalPopup(
        "Error",
        "Call to FbxImporter:: Initialize() failed.\nTry changing your file "
        "name to contain only alphanumeric characters and try again!");
    return false;
  }

  bool ok = importer->Import(mFbxScene);
  if (!ok) {
    DzHelpers::displayCriticalPopup("Error", "importFbxScene failed to import");
  }

  importer->Destroy();
  return ok;
}

void FbxToDazImporter::createAndDisplaySummary(const QString& fbxFilePath,
                                               const QDir& summaryFolder) {
  SummarySingleton* ss = SummarySingleton::getInstance();
  ss->setFbxTextureFolderPath(mFbxTextureFolderPath);

  QString fileName = QFileInfo(fbxFilePath).baseName() % "_import_summary.txt";
  QString path = summaryFolder.filePath(fileName);

  QString shortSummary = ShortSummary(path).create();
  QString longSummary = LongSummary().create();
  QString verboseSummary = VerboseSummary().create();

  DzHelpers::logInfo(verboseSummary);
  bool ok = DzHelpers::writeToFile(path, longSummary);
  if (!ok) {
    DzHelpers::logAndDisplayWarning("Error",
                                    "Failed to write long summary to file.");
  }
  DzHelpers::displayInfoPopup("Import Summary", shortSummary);

  ss->reset();
  ss->terminate();
}
