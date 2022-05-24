#include "SummarySingleton.h"

SummarySingleton* SummarySingleton::mSummarySingleton = 0;
int SummarySingleton::mNormalMappedByVertexCnt = 0;
QString SummarySingleton::mFbxTextureFolderPath = "";
QMap<QString, bool> SummarySingleton::mTextureFileNames = {};
QMap<QString, bool> SummarySingleton::mMaterialNames = {};
QMap<QString, QStringList> SummarySingleton::mNodeNameToMaterials = {};
QList<QPair<QString, QString>> SummarySingleton::mUnknownMaterials = {};
QMap<QString, bool> SummarySingleton::mMissingTextureFiles = {};
QMap<std::tuple<QString, QString, QString>, QString>
    SummarySingleton::mNodeInfoToTextureFile = {};
QMap<std::tuple<QString, QString, QString>, QString>
    SummarySingleton::mNodeInfoToMissingTextureFile = {};
QMap<std::tuple<QString, QString, QString>, QString>
    SummarySingleton::mNodeInfoToUnsupportedTexture = {};
QMap<QString, QString> SummarySingleton::mReplacedTextureFiles = {};

SummarySingleton* SummarySingleton::getInstance() {
  if (!mSummarySingleton) {
    mSummarySingleton = new SummarySingleton();
  }
  return mSummarySingleton;
}

void SummarySingleton::terminate() {
  delete mSummarySingleton;
  mSummarySingleton = nullptr;
}

void SummarySingleton::increaseNormalMappedByVertexCnt() {
  getInstance();
  mNormalMappedByVertexCnt++;
}

void SummarySingleton::reset() {
  getInstance();
  mMaterialNames.clear();
  mNormalMappedByVertexCnt = 0;
  mFbxTextureFolderPath = "";
  mTextureFileNames.clear();
  mNodeNameToMaterials.clear();
  mUnknownMaterials.clear();
  mMissingTextureFiles.clear();
  mNodeInfoToTextureFile.clear();
  mNodeInfoToMissingTextureFile.clear();
  mReplacedTextureFiles.clear();
  mNodeInfoToUnsupportedTexture.clear();
}

void SummarySingleton::setFbxTextureFolderPath(const QDir& textureFolder) {
  mFbxTextureFolderPath = textureFolder.path();
}

void SummarySingleton::addNodeNameMaterial(const QString& nodeName,
                                           const QString& materialName) {
  getInstance();
  if (!mNodeNameToMaterials.contains(nodeName)) {
    mNodeNameToMaterials.insert(nodeName, QStringList());
  }

  mMaterialNames.insert(materialName, false);
  mNodeNameToMaterials[nodeName].append(materialName);
}

void SummarySingleton::addUnknownMaterial(const QString& nodeName,
                                          const QString& materialName) {
  getInstance();
  mUnknownMaterials.append(QPair<QString, QString>{nodeName, materialName});
}

void SummarySingleton::addTextureFileName(const QString& fileName) {
  mTextureFileNames.insert(fileName, false);
}

void SummarySingleton::addNodeInfoToTextureFile(
    const QString& nodeName, const QString& materialName,
    const QString& propertyName, const QString& textureFileName) {
  getInstance();
  std::tuple<QString, QString, QString> nodeInfo =
      std::make_tuple(nodeName, materialName, propertyName);
  mNodeInfoToTextureFile[nodeInfo] = textureFileName;
}

void SummarySingleton::addNodeInfoToMissingTextureFile(
    const QString& nodeName, const QString& materialName,
    const QString& propertyName, const QString& textureFileName) {
  getInstance();
  std::tuple<QString, QString, QString> nodeInfo =
      std::make_tuple(nodeName, materialName, propertyName);
  mMissingTextureFiles.insert(textureFileName, false);
  mNodeInfoToMissingTextureFile[nodeInfo] = textureFileName;
}

void SummarySingleton::addNodeInfoToUnsupportedTexture(
    const QString& nodeName, const QString& materialName,
    const QString& propertyName, const QString& textureName) {
  getInstance();
  std::tuple<QString, QString, QString> nodeInfo =
      std::make_tuple(nodeName, materialName, propertyName);
  mNodeInfoToUnsupportedTexture[nodeInfo] = textureName;
}

void SummarySingleton::addReplacedTextureFile(const QString& origFileName,
                                              const QString& replacedFileName) {
  mReplacedTextureFiles.insert(origFileName, replacedFileName);
}

SummarySingleton::SummarySingleton() { mNormalMappedByVertexCnt = 0; }

SummarySingleton::~SummarySingleton() { mNormalMappedByVertexCnt++; }

// Getters

int SummarySingleton::getNormalMappedByVertexCnt() {
  return mNormalMappedByVertexCnt;
}

QString SummarySingleton::getFbxTextureFolderPath() {
  return mFbxTextureFolderPath;
}

QMap<QString, bool> SummarySingleton::getMaterialNames() {
  return mMaterialNames;
}

QMap<QString, bool> SummarySingleton::getTextureFileNames() {
  return mTextureFileNames;
}

QList<QPair<QString, QString>> SummarySingleton::getUnknownMaterials() {
  return mUnknownMaterials;
}

QMap<QString, QStringList> SummarySingleton::getNodeNameToMaterials() {
  return mNodeNameToMaterials;
}

QMap<QString, bool> SummarySingleton::getMissingTextureFiles() {
  return mMissingTextureFiles;
}

QMap<std::tuple<QString, QString, QString>, QString>
SummarySingleton::getNodeInfoToTextureFile() {
  return mNodeInfoToTextureFile;
}

QMap<std::tuple<QString, QString, QString>, QString>
SummarySingleton::getNodeInfoToMissingTextureFile() {
  return mNodeInfoToMissingTextureFile;
}

QMap<std::tuple<QString, QString, QString>, QString>
SummarySingleton::getUnsupportedTextures() {
  return mNodeInfoToUnsupportedTexture;
}

QMap<QString, QString> SummarySingleton::getReplacedTextureFiles() {
  return mReplacedTextureFiles;
}
