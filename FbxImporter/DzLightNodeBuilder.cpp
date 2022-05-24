#include "DzLightNodeBuilder.h"

DzLightNodeBuilder::DzLightNodeBuilder(FbxNode* root) { mRoot = root; }

DzLightNodeBuilder::~DzLightNodeBuilder() { delete mDzNode; }

void DzLightNodeBuilder::buildNode() {
  FbxLight* fbxLight = mRoot->GetLight();

  int type = fbxLight->LightType.Get();
  FbxDouble intensity = fbxLight->Intensity.Get();
  FbxDouble3 color = fbxLight->Color.Get();

  DzDistantLight* mDzLightNode;

  switch (type) {
    case 0:
      mDzLightNode = new DzPointLight();
      setColorIntensity(mDzLightNode, intensity, color);
      break;
    case 1:
      mDzLightNode = new DzSpotLight();
      setColorIntensity(mDzLightNode, intensity, color);
      break;
    case 2:
      mDzLightNode = new DzSpotLight();
      setColorIntensity(mDzLightNode, intensity, color);
      break;
    default:
      mDzLightNode = new DzDistantLight();
  }

  mDzNode = mDzLightNode;

  mDzNode->setName(mRoot->GetName());
  mDzNode->setLabel(mDzNode->getName());
  setNodeProperties(mDzNode);
}

void DzLightNodeBuilder::setColorIntensity(DzDistantLight* light,
                                           FbxDouble intensity,
                                           FbxDouble3 color) {
  light->getIntensityControl()->setValue(intensity);
  QColor qcolor = QColor(round(color[0] * 255), round(color[1] * 255),
                         round(color[2] * 255));
  light->getDiffuseColorControl()->setColorValue(qcolor);
}

FbxString DzLightNodeBuilder::getLightTypeName(FbxLight::EType type) {
  switch (type) {
    case 0:
      return "Point";
    case 1:
      return "Directional";
    case 2:
      return "Spot";
    case 3:
      return "Area";
    case 4:
      return "Volumetric";
    default:
      return "unknown";
  }
}