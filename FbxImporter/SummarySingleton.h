#pragma once

#include "DzHelpers.h"

class SummarySingleton {
 public:
  static SummarySingleton* getInstance();

  void terminate();

  static void addNodeNameMaterial(const QString& nodeName,
                                  const QString& materialName);
  static void addUnknownMaterial(const QString& nodeName,
                                 const QString& materialName);
  static void addTextureFileName(const QString& fileName);
  static void addNodeInfoToTextureFile(const QString& nodeName,
                                       const QString& materialName,
                                       const QString& propertyName,
                                       const QString& textureFileName);
  static void addNodeInfoToMissingTextureFile(const QString& nodeName,
                                              const QString& materialName,
                                              const QString& propertyName,
                                              const QString& textureFileName);
  static void addNodeInfoToUnsupportedTexture(const QString& nodeName,
                                              const QString& materialName,
                                              const QString& propertyName,
                                              const QString& textureName);
  static void addReplacedTextureFile(const QString& origFileName,
                                     const QString& replacedFileName);
  static void increaseNormalMappedByVertexCnt();
  static void reset();
  static void setFbxTextureFolderPath(const QDir& textureFolder);

  // Getters
  static int getNormalMappedByVertexCnt();
  static QString getFbxTextureFolderPath();
  static QMap<QString, bool> getMaterialNames();
  static QMap<QString, bool> getTextureFileNames();
  static QList<QPair<QString, QString>> getUnknownMaterials();
  static QMap<QString, QStringList> getNodeNameToMaterials();
  static QMap<QString, bool> getMissingTextureFiles();
  static QMap<std::tuple<QString, QString, QString>, QString>
  getNodeInfoToTextureFile();
  static QMap<std::tuple<QString, QString, QString>, QString>
  getNodeInfoToMissingTextureFile();
  static QMap<std::tuple<QString, QString, QString>, QString>
  getUnsupportedTextures();
  static QMap<QString, QString> getReplacedTextureFiles();

 private:
  static SummarySingleton* mSummarySingleton;

  SummarySingleton();
  ~SummarySingleton();

  // We use a QMap<QString, bool> in some places because it behaves like an
  // ordered set
  static int mNormalMappedByVertexCnt;
  static QString mFbxTextureFolderPath;
  static QMap<QString, bool> mMaterialNames;
  static QMap<QString, bool> mTextureFileNames;
  static QList<QPair<QString, QString>> mUnknownMaterials;
  static QMap<QString, QStringList> mNodeNameToMaterials;
  static QMap<QString, bool> mMissingTextureFiles;
  static QMap<std::tuple<QString, QString, QString>, QString>
      mNodeInfoToTextureFile;
  static QMap<std::tuple<QString, QString, QString>, QString>
      mNodeInfoToMissingTextureFile;
  static QMap<std::tuple<QString, QString, QString>, QString>
      mNodeInfoToUnsupportedTexture;
  static QMap<QString, QString> mReplacedTextureFiles;
};
