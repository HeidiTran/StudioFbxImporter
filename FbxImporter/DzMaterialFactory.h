#pragma once

#include "DzHelpers.h"
#include "MTLParser.h"
#include "MaterialPropertyEnum.h"
#include "SummarySingleton.h"
#include "dzapp.h"
#include "dzcolorproperty.h"
#include "dzdefaultmaterial.h"
#include "dzfloatproperty.h"
#include "dzimagemgr.h"
#include "dzimageproperty.h"
#include "dzmaterial.h"

class DzMaterialFactory {
 public:
  DzMaterialFactory(QDir fbxTextureFolderPath,
                    const QMap<QString, MTLMap>& materialNameToMTLMap);
  ~DzMaterialFactory();

  DzMaterial* createMaterial(QString fbxNodeName,
                             FbxSurfaceMaterial* fbxMaterial);
  DzMaterial* createDefaultMaterial();

 private:
  QString mFbxNodeName;
  QString mFbxMaterialName;
  QDir mFbxTextureFolderPath;
  QMap<QString, MTLMap> mMaterialNameToMTLMap;

  DzMaterial* handleLambertMat(FbxSurfaceMaterial* fbxMaterial);
  DzMaterial* handlePhongMat(FbxSurfaceMaterial* fbxMaterial);
  DzMaterial* handleUnknownMat(FbxSurfaceMaterial* fbxMaterial);
  const FbxImplementation* lookForImplementation(FbxSurfaceMaterial* pMaterial);
  DzTexture* toTexture(const FbxProperty& fbxProperty);
  DzTexture* handleFileTexture(const fbxsdk::FbxFileTexture* texture,
                               const QString& propertyName);
  QColor toQColor(const FbxPropertyT<FbxDouble3>& fbxProperty);
  QColor toQColor(float r, float g, float b);
  QColor toQColor(const FbxColor& fbxColor);
  DzTexture* importTextureFile(const QString& fullFilePath);
  QFileInfo getFileWithSameName(QFileInfo origFile);
  void applyMTLProperties(DzMaterial* dzMaterial);
};
