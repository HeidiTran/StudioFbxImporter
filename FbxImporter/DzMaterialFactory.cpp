#include "DzMaterialFactory.h"

DzMaterialFactory::DzMaterialFactory(
    QDir fbxTextureFolderPath,
    const QMap<QString, MTLMap>& materialNameToMTLMap) {
  mFbxTextureFolderPath = fbxTextureFolderPath;
  mMaterialNameToMTLMap = materialNameToMTLMap;
}

DzMaterialFactory::~DzMaterialFactory() {}

DzMaterial* DzMaterialFactory::createMaterial(QString fbxNodeName,
                                              FbxSurfaceMaterial* fbxMaterial) {
  mFbxNodeName = fbxNodeName;
  mFbxMaterialName = fbxMaterial->GetName();

  QString fbxMatName = fbxMaterial->GetName();
  SummarySingleton::addNodeNameMaterial(mFbxNodeName, fbxMatName);

  // An FBX material is either a hardware shader, phong, lambert, or some
  // unknown type.
  const FbxImplementation* fbxImplementation =
      lookForImplementation(fbxMaterial);
  bool isHardwareShader = fbxImplementation != NULL;
  bool isPhongMaterial = fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId);
  bool isLambertMaterial =
      fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId);

  DzMaterial* dzMaterial = new DzDefaultMaterial();
  if (isHardwareShader) {
    DzHelpers::logAndDisplayWarning("FbxImporter Warning",
                                    "Hardware shader was found.");
  } else if (isPhongMaterial) {
    dzMaterial = handlePhongMat(fbxMaterial);
  } else if (isLambertMaterial) {
    dzMaterial = handleLambertMat(fbxMaterial);
  } else {
    dzMaterial = handleUnknownMat(fbxMaterial);
  }

  applyMTLProperties(dzMaterial);

  return dzMaterial;
}

DzMaterial* DzMaterialFactory::createDefaultMaterial() {
  DzMaterial* dzMaterial = new DzDefaultMaterial();
  dzMaterial->setName("DazDefaultMaterial");
  dzMaterial->setLabel(dzMaterial->getName());
  return dzMaterial;
}

DzMaterial* DzMaterialFactory::handleLambertMat(
    FbxSurfaceMaterial* fbxMaterial) {
  DzMaterial* dzMaterial = new DzDefaultMaterial();
  DzDefaultMaterial* dzDefMaterial =
      qobject_cast<DzDefaultMaterial*>(dzMaterial);
  dzMaterial->setName(mFbxMaterialName);

  FbxSurfaceLambert* lambertMaterial = (FbxSurfaceLambert*)fbxMaterial;

  // Diffuse
  QColor dzDiffuseColor = toQColor(lambertMaterial->Diffuse);
  DzTexture* dzDiffuseMap = toTexture(lambertMaterial->Diffuse);
  dzMaterial->setDiffuseColor(dzDiffuseColor);
  dzMaterial->setColorMap(dzDiffuseMap);

  // Opacity
  // TODO: Trouble getting the correct opacity value
  // float dzBaseOpacity = lambertMaterial->TransparencyFactor.Get();
  float dzBaseOpacity = 1 - ((lambertMaterial->TransparentColor.Get()[0] +
                              lambertMaterial->TransparentColor.Get()[1] +
                              lambertMaterial->TransparentColor.Get()[2]) /
                             3);
  // dzMaterial->setBaseOpacity(dzBaseOpacity);

  DzTexture* dzOpacityMap = toTexture(lambertMaterial->TransparentColor);
  dzMaterial->setOpacityMap(dzOpacityMap);

  // Emissive
  // TODO: How to set emissive color in Daz?
  QColor dzEmissiveColor = toQColor(lambertMaterial->Emissive);

  if (dzDefMaterial) {
    // Ambient
    QColor dzAmbientColor = toQColor(lambertMaterial->Ambient);
    DzTexture* dzAmbientMap = toTexture(lambertMaterial->Ambient);
    float dzAmbientFactor = lambertMaterial->AmbientFactor.Get();
    dzDefMaterial->setAmbientColor(dzAmbientColor);
    dzDefMaterial->setAmbientColorMap(dzAmbientMap);
    dzDefMaterial->setAmbientStrength(dzAmbientFactor);

    // Normal Map
    DzTexture* dzNormalMap = toTexture(lambertMaterial->NormalMap);
    dzDefMaterial->setNormalValueMap(dzNormalMap);

    // Bump
    DzTexture* dzBumpMap = toTexture(lambertMaterial->Bump);
    float dzBumpFactor = lambertMaterial->BumpFactor.Get();
    dzDefMaterial->setBumpMap(dzBumpMap);
    dzDefMaterial->setBumpStrength(dzBumpFactor);

    // Displacement
    float dzDisplacementFactor =
        lambertMaterial->DisplacementFactor.Get() / 100;
    dzDefMaterial->setDisplacementStrength(dzDisplacementFactor);
    DzTexture* dzDisplacementMap =
        toTexture(lambertMaterial->DisplacementColor);
    dzDefMaterial->setDisplacementMap(dzDisplacementMap);
  }

  return dzMaterial;
}

DzMaterial* DzMaterialFactory::handlePhongMat(FbxSurfaceMaterial* fbxMaterial) {
  DzMaterial* dzMaterial = NULL;
  const DzClassFactory* pbrMatFactory =
      dzApp->findClassFactory("DzPbrMaterial");
  if (QObject* pbrMatInstance =
          pbrMatFactory ? pbrMatFactory->createInstance() : NULL) {
    dzMaterial = qobject_cast<DzMaterial*>(pbrMatInstance);
  } else {
    dzMaterial = new DzDefaultMaterial();
  }

  FbxSurfacePhong* phongMaterial = (FbxSurfacePhong*)fbxMaterial;
  dzMaterial->setName(mFbxMaterialName);
  DzDefaultMaterial* dzDefMaterial =
      qobject_cast<DzDefaultMaterial*>(dzMaterial);

  // Diffuse
  QColor dzDiffuseColor = toQColor(phongMaterial->Diffuse);
  DzTexture* dzDiffuseMap = toTexture(phongMaterial->Diffuse);
  dzMaterial->setDiffuseColor(dzDiffuseColor);
  dzMaterial->setColorMap(dzDiffuseMap);

  // TODO: Opacity not correct
  // Opacity (aka transparency)
  // float dzBaseOpacity = phongMaterial->TransparencyFactor.Get();
  float dzBaseOpacity = 1 - phongMaterial->TransparencyFactor.Get() *
                                ((phongMaterial->TransparentColor.Get()[0] +
                                  phongMaterial->TransparentColor.Get()[1] +
                                  phongMaterial->TransparentColor.Get()[2]) /
                                 3);
  // dzMaterial->setBaseOpacity(dzBaseOpacity);

  DzTexture* dzOpacityMap = toTexture(phongMaterial->TransparentColor);
  dzMaterial->setOpacityMap(dzOpacityMap);

  if (dzDefMaterial) {
    // Ambient
    QColor dzAmbientColor = toQColor(phongMaterial->Ambient);
    DzTexture* dzAmbientMap = toTexture(phongMaterial->Ambient);
    float dzAmbientFactor = phongMaterial->AmbientFactor.Get();
    dzDefMaterial->setAmbientColor(dzAmbientColor);
    dzDefMaterial->setAmbientColorMap(dzAmbientMap);
    dzDefMaterial->setAmbientStrength(dzAmbientFactor);

    // Diffuse factor
    float diffuseFactor = phongMaterial->DiffuseFactor.Get();
    dzDefMaterial->setDiffuseStrength(diffuseFactor);

    // Specular
    QColor dzSpecularColor = toQColor(phongMaterial->Specular);
    DzTexture* dzSpecularMap = toTexture(phongMaterial->Specular);
    float dzSpecularFactor = phongMaterial->SpecularFactor.Get();
    dzDefMaterial->setSpecularColor(dzSpecularColor);
    dzDefMaterial->setSpecularColorMap(dzSpecularMap);
    dzDefMaterial->setSpecularStrength(dzSpecularFactor);

    // Shininess
    // We divide by 100 since FBX Shininess already in percentage
    float dzShininess = phongMaterial->Shininess.Get() / 100;
    DzTexture* dzShininessMap = toTexture(phongMaterial->Shininess);
    dzDefMaterial->setGlossinessStrength(dzShininess);
    dzDefMaterial->setGlossinessValueMap(dzShininessMap);

    // Reflectivity
    float dzReflectivity = phongMaterial->ReflectionFactor;
    DzTexture* dzReflectivityMap = toTexture(phongMaterial->ReflectionFactor);
    dzDefMaterial->setReflectionStrength(dzReflectivity);
    dzDefMaterial->setReflectionMap(dzReflectivityMap);

    // Normal Map
    DzTexture* dzNormalMap = toTexture(phongMaterial->NormalMap);
    dzDefMaterial->setNormalValueMap(dzNormalMap);

    // Bump Map
    DzTexture* dzBumpMap = toTexture(phongMaterial->Bump);
    float dzBumpFactor = phongMaterial->BumpFactor.Get();
    dzDefMaterial->setBumpMap(dzBumpMap);
    dzDefMaterial->setBumpStrength(dzBumpFactor);

    // Displacement
    DzTexture* dzDisplacementMap = toTexture(phongMaterial->DisplacementColor);
    dzDefMaterial->setDisplacementMap(dzDisplacementMap);
    // We divide by 100 since FBX displacement factor already in percentage
    float dzDisplacementFactor = phongMaterial->DisplacementFactor.Get() / 100;
    dzDefMaterial->setDisplacementStrength(dzDisplacementFactor);
  } else {  // DzPbrMaterial or DzUberIrayMaterial
    // Roughness
    float dzRoughness =
        1.0 - ((log(phongMaterial->Shininess.Get()) / log(2.0)) - 2) / 10;
    if (!QMetaObject::invokeMethod(dzMaterial, "setRoughness",
                                   Q_ARG(float, dzRoughness))) {
      DzFloatProperty* fProp = qobject_cast<DzFloatProperty*>(
          dzMaterial->findProperty("Glossy Roughness"));
      if (fProp) {
        fProp->setValue(dzRoughness);
      }
    }

    // Metallicity
    FbxDouble3 spec = phongMaterial->Specular.Get();
    float innerDistance = qAbs(spec[1] - spec[0]) + qAbs(spec[2] - spec[0]) +
                          qAbs(spec[2] - spec[1]);
    float dzMetallicity = qMin(1.0f, innerDistance);
    // use "setMetallicity" double if using DzUberIrayMaterial
    if (!QMetaObject::invokeMethod(dzMaterial, "setMetallicity",
                                   Q_ARG(float, dzMetallicity))) {
      DzFloatProperty* fProp = qobject_cast<DzFloatProperty*>(
          dzMaterial->findProperty("Metallic Weight"));
      if (fProp) {
        fProp->setValue(dzMetallicity);
      }
    }

    // Normal Map
    if (DzImageProperty* normalMap = qobject_cast<DzImageProperty*>(
            dzMaterial->findProperty("Normal Map"))) {
      normalMap->setValue(toTexture(phongMaterial->NormalMap));
    }

    // Base Bump
    if (DzImageProperty* bumpMap = qobject_cast<DzImageProperty*>(
            dzMaterial->findProperty("Base Bump"))) {
      bumpMap->setValue(toTexture(phongMaterial->Bump));
    }

    // Emissive Color
    QColor dzEmissiveColor = toQColor(phongMaterial->Emissive);
    if (DzColorProperty* emissionColor = qobject_cast<DzColorProperty*>(
            dzMaterial->findProperty("Emission Color"))) {
      emissionColor->setColorValue(dzEmissiveColor);
    }
  }

  return dzMaterial;
}

DzMaterial* DzMaterialFactory::handleUnknownMat(
    FbxSurfaceMaterial* fbxMaterial) {
  QString dzMaterialName = fbxMaterial->GetName();
  DzMaterial* dzMaterial = new DzDefaultMaterial();
  dzMaterial->setName(dzMaterialName);

  DzDefaultMaterial* dzDefMaterial =
      qobject_cast<DzDefaultMaterial*>(dzMaterial);

  FbxProperty curProp = fbxMaterial->GetFirstProperty();
  while (curProp.IsValid()) {
    FbxDataType propDT = curProp.GetPropertyDataType();
    FbxString propName = curProp.GetName();

    if (propDT.GetType() == eFbxDouble || propDT.GetType() == eFbxFloat) {
      float value = curProp.Get<FbxDouble>();
      if (isMatProp(propName, AmbientFactor)) {
        dzDefMaterial->setAmbientStrength(value);
      } else if (isMatProp(propName, DiffuseFactor)) {
        dzDefMaterial->setDiffuseStrength(value);
      } else if (isMatProp(propName, DisplacementFactor)) {
        // We divide by 100 since FBX displacement factor already in percentage
        dzDefMaterial->setDisplacementStrength(value / 100);
      } else if (isMatProp(propName, Reflectivity)) {
        dzDefMaterial->setReflectionStrength(value);
      } else if (isMatProp(propName, SpecularFactor)) {
        dzDefMaterial->setSpecularStrength(value);
      } else if (isMatProp(propName, ShininessExponent)) {
        // We divide by 100 since FBX Shininess already in percentage
        dzDefMaterial->setGlossinessStrength(value / 100);
      }
    } else if (propDT.Is(FbxColor3DT) || propDT.Is(FbxColor4DT)) {
      FbxColor fbxColor = curProp.Get<FbxColor>();
      QColor color = toQColor(fbxColor);

      if (isMatProp(propName, DiffuseColor)) {
        dzMaterial->setDiffuseColor(color);
      }

      if (dzDefMaterial) {
        if (isMatProp(propName, AmbientColor)) {
          dzDefMaterial->setAmbientColor(color);
        } else if (isMatProp(propName, SpecularColor)) {
          dzDefMaterial->setSpecularColor(color);
        }
      }
    }

    curProp = fbxMaterial->GetNextProperty(curProp);
  }

  for (int layerIdx = 0, textureIdx = 0;
       layerIdx < FbxLayerElement::sTypeTextureCount; layerIdx++) {
    FbxProperty property = fbxMaterial->FindProperty(
        FbxLayerElement::sTextureChannelNames[textureIdx]);

    if (property.IsValid()) {
      int textureCnt = property.GetSrcObjectCount<FbxTexture>();
      for (int j = 0; j < textureCnt; ++j) {
        // Here we have to check if it's layeredtextures, or just textures:
        FbxLayeredTexture* layeredTexture =
            property.GetSrcObject<FbxLayeredTexture>(j);
        if (layeredTexture) {
          // TODO: Handle layered texture. Test file is ISDBridge
          DzHelpers::logWarning("Is layered texture");
        } else {
          FbxString propName = property.GetName();
          DzTexture* texture = toTexture(property);

          // no layered texture simply get on the property
          if (isMatProp(propName, DiffuseColor)) {
            dzMaterial->setColorMap(texture);
          }

          if (dzDefMaterial) {
            if (isMatProp(propName, AmbientColor)) {
              dzDefMaterial->setAmbientColorMap(texture);
            } else if (isMatProp(propName, SpecularColor)) {
              dzDefMaterial->setSpecularColorMap(texture);
            } else if (isMatProp(propName, ShininessExponent)) {
              dzDefMaterial->setGlossinessValueMap(texture);
            } else if (isMatProp(propName, NormalMap)) {
              dzDefMaterial->setNormalValueMap(texture);
            } else if (isMatProp(propName, Bump)) {
              dzDefMaterial->setBumpMap(texture);
            } else if (isMatProp(propName, DisplacementColor)) {
              dzDefMaterial->setDisplacementMap(texture);
            } else if (isMatProp(propName, ReflectionFactor)) {
              dzDefMaterial->setReflectionMap(texture);
            }
          }
        }
      }
    }
  }

  SummarySingleton::addUnknownMaterial(mFbxNodeName, dzMaterialName);
  return dzMaterial;
}

const FbxImplementation* DzMaterialFactory::lookForImplementation(
    FbxSurfaceMaterial* pMaterial) {
  const FbxImplementation* imp = nullptr;
  if (!imp) imp = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_CGFX);
  if (!imp) imp = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_HLSL);
  if (!imp) imp = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SFX);
  if (!imp) imp = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_OGS);
  if (!imp) imp = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_SSSL);
  return imp;
}

QColor DzMaterialFactory::toQColor(
    const FbxPropertyT<FbxDouble3>& fbxProperty) {
  QColor c;
  FbxDouble3 fbxRgbColor = fbxProperty.Get();

  c.setRedF(fbxRgbColor[0]);
  c.setGreenF(fbxRgbColor[1]);
  c.setBlueF(fbxRgbColor[2]);

  return c;
}

QColor DzMaterialFactory::toQColor(const FbxColor& fbxColor) {
  QColor c;
  c.setRedF(fbxColor.mRed);
  c.setGreenF(fbxColor.mGreen);
  c.setBlueF(fbxColor.mBlue);
  c.setAlphaF(fbxColor.mAlpha);
  return c;
}

/// <summary>
/// r, g, and b are in [0, 1]
/// </summary>
QColor DzMaterialFactory::toQColor(float r, float g, float b) {
  QColor c;

  c.setRedF(r);
  c.setGreenF(g);
  c.setBlueF(b);

  return c;
}

DzTexture* DzMaterialFactory::toTexture(const FbxProperty& fbxProperty) {
  if (!fbxProperty.IsValid()) {
    DzHelpers::logError("Property not valid!");
    DzHelpers::logError(fbxProperty.GetName());
    return nullptr;
  }

  int textureCnt = fbxProperty.GetSrcObjectCount<FbxTexture>();

  for (int i = 0; i < textureCnt; i++) {
    FbxLayeredTexture* layeredTexture =
        fbxProperty.GetSrcObject<FbxLayeredTexture>(i);
    if (layeredTexture) {
      // TODO: Support layered texture
      DzHelpers::logWarning("Is layered texture");
    } else {
      FbxTexture* texture = fbxProperty.GetSrcObject<FbxTexture>(i);
      QString propertyName = fbxProperty.GetName().Buffer();
      FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
      FbxProceduralTexture* proceduralTexture =
          FbxCast<FbxProceduralTexture>(texture);

      if (fileTexture) {
        return handleFileTexture(fileTexture, propertyName);
      } else if (proceduralTexture) {
        // TODO: Support procedural texture
        DzHelpers::logError("Procedural Texture found!");
        DzHelpers::logWarning(propertyName);
      }
    }
  }

  return nullptr;
}

DzTexture* DzMaterialFactory::handleFileTexture(
    const fbxsdk::FbxFileTexture* texture, const QString& propertyName) {
  if (*texture->GetFileName() == NULL) {
    SummarySingleton::addNodeInfoToUnsupportedTexture(
        mFbxNodeName, mFbxMaterialName, propertyName, texture->GetName());

    // Print out all the properties of this texture
    // FbxProperty curProp = texture->GetFirstProperty();
    // while (curProp.IsValid()) {
    //  FbxDataType propDT = curProp.GetPropertyDataType();
    //  FbxString propName = curProp.GetName();

    //  if (propDT.Is(FbxColor3DT) || propDT.Is(FbxColor4DT)) {
    //    DzHelpers::logInfo(propName);
    //    FbxColor fbxColor = curProp.Get<FbxColor>();
    //    QColor color = toQColor(fbxColor);
    //    // Set diffuse color of the material map to this texture
    //  }

    //  curProp = texture->GetNextProperty(curProp);
    //}
    return nullptr;
  }

  QFileInfo textureFilePath = QString::fromUtf8(texture->GetFileName());

  // Texture file path in the FBX's file might not be the correct path
  QString actualTextureFilePath =
      mFbxTextureFolderPath.filePath(textureFilePath.fileName());

  DzTexture* dsTexture = importTextureFile(actualTextureFilePath);
  SummarySingleton::addTextureFileName(actualTextureFilePath);

  if (!dsTexture) {
    QFileInfo replacedFilePath = getFileWithSameName(textureFilePath);

    if (replacedFilePath.fileName() != textureFilePath.fileName()) {
      dsTexture = importTextureFile(
          mFbxTextureFolderPath.filePath(replacedFilePath.fileName()));
      SummarySingleton::addReplacedTextureFile(textureFilePath.fileName(),
                                               replacedFilePath.fileName());
    }
  }

  SummarySingleton::addNodeInfoToTextureFile(
      mFbxNodeName, mFbxMaterialName, propertyName, textureFilePath.fileName());
  if (!dsTexture) {
    SummarySingleton::addNodeInfoToMissingTextureFile(
        mFbxNodeName, mFbxMaterialName, propertyName,
        textureFilePath.fileName());
  }

  return dsTexture;
}

DzTexture* DzMaterialFactory::importTextureFile(const QString& fullFilePath) {
  if (QFileInfo(fullFilePath).exists() && QFileInfo(fullFilePath).isFile()) {
    return dzApp->getImageMgr()->getImage(fullFilePath);
  }

  return nullptr;
}

/* Return the original file if no other files with the same name is found
 */
QFileInfo DzMaterialFactory::getFileWithSameName(QFileInfo origFile) {
  QMap<QString, QString> commonExtReplacement;
  commonExtReplacement["jpg"] = "jpeg";
  commonExtReplacement["jpeg"] = "jpg";

  QString origFileExt = origFile.suffix();
  QString origFileBaseName = origFile.baseName();

  if (commonExtReplacement.contains(origFileExt)) {
    QString replacedFileName =
        origFileBaseName + "." + commonExtReplacement[origFileExt];
    QFileInfo replacedFile = mFbxTextureFolderPath.filePath(replacedFileName);
    if (replacedFile.exists()) {
      return origFileBaseName + "." + commonExtReplacement[origFileExt];
    }
  }

  QStringList nameFilters;
  nameFilters << origFileBaseName + ".*";
  QFileInfoList allFilesWithSameName =
      mFbxTextureFolderPath.entryInfoList(nameFilters);

  for (int i = 0; i < allFilesWithSameName.size(); i++) {
    if (allFilesWithSameName[i].fileName() != origFile.fileName()) {
      return allFilesWithSameName[i];
    }
  }

  return origFile;
}

void DzMaterialFactory::applyMTLProperties(DzMaterial* dzMaterial) {
  if (!mMaterialNameToMTLMap.contains(mFbxMaterialName)) {
    return;
  }

  auto importTextureFileWithRetry = [&](QString filepath,
                                        QString retryPath) -> DzTexture* {
    DzTexture* texture = importTextureFile(filepath);
    if (texture) {
      return texture;
    }
    texture = importTextureFile(retryPath);
    if (!texture) {
      DzHelpers::logWarning("Failed to import " + filepath + " and tried " +
                            retryPath);
    }
    return texture;
  };

  DzDefaultMaterial* dzDefMaterial =
      qobject_cast<DzDefaultMaterial*>(dzMaterial);
  MTLMap map = mMaterialNameToMTLMap[mFbxMaterialName];
  auto iter = map.constBegin();
  while (iter != map.constEnd()) {
    MTLKey k = iter.key();
    MTLValue v = iter.value();

    // TODO: Currently, we're just bringing in the map files because they are
    // the most important and impactful properties to integrate into the
    // converter, but we should try to complete the MTL parser integration at
    // some point. Some of the code below is commented out because it doesn't
    // seem to work properly
    if (k == MTLKey::Ka && dzDefMaterial) {
      // QList<float> rgb = v.toQListFloat();
      // dzDefMaterial->setAmbientColor(toQColor(rgb[0], rgb[1], rgb[2]));
    } else if (k == MTLKey::Kd) {
      // QList<float> rgb = v.toQListFloat();
      // dzMaterial->setDiffuseColor(toQColor(rgb[0], rgb[1], rgb[2]));
    } else if (k == MTLKey::Ks && dzMaterial) {
      // QList<float> rgb = v.toQListFloat();
      // dzDefMaterial->setSpecularColor(toQColor(rgb[0], rgb[1], rgb[2]));
    } else if (k == MTLKey::Ke) {
      // TODO: Support Ke
    } else if (k == MTLKey::Ns) {
      // TODO: Support Ns
    } else if (k == MTLKey::Ni) {
      // TODO: Support Ni
    } else if (k == MTLKey::d) {
      // TODO: Support d
    } else if (k == MTLKey::illum) {
      // TODO: Support illum
    } else if (k == MTLKey::map_Kd) {
      QString filepath = mFbxTextureFolderPath.filePath(v.toQString());
      QString retryPath =
          mFbxTextureFolderPath.filePath(QFileInfo(v.toQString()).fileName());
      DzTexture* texture = importTextureFileWithRetry(filepath, retryPath);
      dzMaterial->setColorMap(texture);
    } else if (k == MTLKey::map_Bump && dzDefMaterial) {
      QString filepath = mFbxTextureFolderPath.filePath(v.toQString());
      QString retryPath =
          mFbxTextureFolderPath.filePath(QFileInfo(v.toQString()).fileName());
      DzTexture* texture = importTextureFileWithRetry(filepath, retryPath);
      dzDefMaterial->setBumpMap(texture);
    } else if (k == MTLKey::map_Ks && dzDefMaterial) {
      QString filepath = mFbxTextureFolderPath.filePath(v.toQString());
      QString retryPath =
          mFbxTextureFolderPath.filePath(QFileInfo(v.toQString()).fileName());
      DzTexture* texture = importTextureFileWithRetry(filepath, retryPath);
      dzDefMaterial->setSpecularColorMap(texture);
    } else if (k == MTLKey::map_Ke) {
      // TODO: Support map_Ke
    } else if (k == MTLKey::map_d) {
      // TODO: Support map_d
      // map_d doesn't work atm because opacity doesn't seem to convert overall
    }

    iter++;
  }
}