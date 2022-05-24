#include "VerboseSummary.h"

VerboseSummary::VerboseSummary() {}

VerboseSummary::~VerboseSummary() {}

void VerboseSummary::createMissingTextureFilesSection() {
  auto nodeInfoToMissingTextureFile =
      SummarySingleton::getNodeInfoToMissingTextureFile();
  if (!nodeInfoToMissingTextureFile.isEmpty()) {
    mSummary = mSummary % "\n- Missing textures\n";
    auto iter = nodeInfoToMissingTextureFile.constBegin();
    while (iter != nodeInfoToMissingTextureFile.constEnd()) {
      QString nodeName = std::get<0>(iter.key());
      QString materialName = std::get<1>(iter.key());
      QString propertyName = std::get<2>(iter.key());
      mSummary = mSummary % "Node: " % nodeName % " (" % materialName;
      mSummary =
          mSummary % ": " % propertyName % "), File: " % iter.value() % "\n";
      iter++;
    }
  }
}

void VerboseSummary::sectionHook() {
  createMaterialNamesSection();
  createNodeNameToMaterialsSection();
  createNodeInfoToTextureFileSection();
}

void VerboseSummary::createMaterialNamesSection() {
  QStringList materialNames = SummarySingleton::getMaterialNames().keys();
  if (!materialNames.isEmpty()) {
    mSummary = mSummary % "\n- " +
               QString::number(materialNames.size()) % " materials in total: ";
    mSummary = mSummary % materialNames.join(", ") % "\n";
  }
}

void VerboseSummary::createNodeNameToMaterialsSection() {
  auto nodeNameToMaterials = SummarySingleton::getNodeNameToMaterials();
  if (!nodeNameToMaterials.isEmpty()) {
    mSummary = mSummary % "\n- Node - Materials\n";
    auto iter = nodeNameToMaterials.constBegin();
    while (iter != nodeNameToMaterials.constEnd()) {
      mSummary = mSummary % iter.key() % ": " % iter.value().join(", ") % "\n";
      iter++;
    }
  }
}

void VerboseSummary::createNodeInfoToTextureFileSection() {
  auto nodeInfoToTextureFile = SummarySingleton::getNodeInfoToTextureFile();
  if (!nodeInfoToTextureFile.isEmpty()) {
    mSummary = mSummary % "\n- Node Name - Texture File Name\n";
    auto iter = nodeInfoToTextureFile.constBegin();
    while (iter != nodeInfoToTextureFile.constEnd()) {
      QString nodeName = std::get<0>(iter.key());
      QString materialName = std::get<1>(iter.key());
      QString propertyName = std::get<2>(iter.key());
      mSummary = mSummary % "Node: " % nodeName % " (" % materialName;
      mSummary =
          mSummary % ": " % propertyName % "), File: " % iter.value() % "\n";
      iter++;
    }
  }
}
