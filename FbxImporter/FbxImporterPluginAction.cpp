#include "FbxImporterPluginAction.h"

#include "moc_FbxImporterPluginAction.cpp"

DzFbxImporterAction::DzFbxImporterAction()
    : DzAction(tr("Import"),
               tr("This option will import an FBX file into Daz")) {}

// This is the entry point of our plugin
void DzFbxImporterAction::executeAction() {
  DzHelpers::logInfo("=================== BEGIN ===================");

  QString fbxFilePath = nullptr;
  bool ok = DzHelpers::getFbxFilePath(fbxFilePath, "Select an FBX file",
                                      "Autodesk (*.fbx)");
  if (!ok) {
    DzHelpers::logAndDisplayCritical("FBX Importer",
                                     "Failed to get FBX file path!");
    return;
  }

  if (fbxFilePath == nullptr) {
    DzHelpers::displayInfoPopup("FBX Importer", "No file was selected.");
    return;
  }

  QDir fbxSourceFolder = fbxFilePath;
  fbxSourceFolder.cdUp();

  QDir fbxTextureFolderPath = fbxSourceFolder;
  const QString SELECT_TEXTURE_FOLDER_PROMPT =
      "Select the folder containing the FBX file's textures. Cancel if there "
      "is no texture folder.";
  DzHelpers::getFolderPath(fbxFilePath, fbxTextureFolderPath,
                           SELECT_TEXTURE_FOLDER_PROMPT);

  QDir summaryFolderPath = fbxSourceFolder;
  const QString SELECT_SUMMARY_FOLDER_PROMPT =
      "Select the folder for the output summary to be written to.";
  ok = DzHelpers::getFolderPath(fbxFilePath, summaryFolderPath,
                                SELECT_SUMMARY_FOLDER_PROMPT);
  if (!ok) {
    DzHelpers::logAndDisplayInfo("FBX Importer",
                                 "Failed to get summary folder path!");
    return;
  }

  QString mtlFilePath = nullptr;
  ok = DzHelpers::getFbxFilePath(
      mtlFilePath, "Select an MTL file. Cancel if there is no MTL file.",
      "MTL File (*.mtl)");
  if (!ok) {
    DzHelpers::logAndDisplayCritical("FBX Importer",
                                     "Failed to get MTL file path!");
  }

  DzHelpers::logInfo("Importing " + fbxFilePath);
  FbxToDazImporter fbxToDazImporter = FbxToDazImporter();
  fbxToDazImporter.convert(fbxFilePath, fbxTextureFolderPath, summaryFolderPath,
                           mtlFilePath);
}
