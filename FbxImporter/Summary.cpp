#include "Summary.h"

Summary::Summary() { mSummary = ""; }

Summary::~Summary() {}

QString Summary::create() {
  createHeader();
  createNormalMappedByVertexSection();
  createUnknownMaterialsSection();
  createMissingTextureFilesSection();
  createReplacedTextureFilesSection();
  createUnsupportedTexturesSection();
  sectionHook();
  createNotSupportedSection();
  footerHook();

  return mSummary;
}

void Summary::createHeader() {
  int numNodes = SummarySingleton::getNodeNameToMaterials().size();
  int numMaterials = SummarySingleton::getMaterialNames().size();
  int numTextures = SummarySingleton::getTextureFileNames().size();

  mSummary = mSummary % QString::number(numNodes) %
             ((numNodes == 1) ? " Node, " : " Nodes, ");
  mSummary = mSummary % QString::number(numMaterials) %
             ((numMaterials == 1) ? " Material, " : " Materials, ");
  mSummary =
      mSummary % QString::number(numTextures) %
      ((numTextures == 1) ? " Texture imported\n" : " Textures imported\n");
}

void Summary::createNormalMappedByVertexSection() {
  int numUnsupportedNormals = SummarySingleton::getNormalMappedByVertexCnt();
  if (numUnsupportedNormals > 0) {
    mSummary = mSummary % "\n- " % QString::number(numUnsupportedNormals) %
               " normals mapped by vertex not supported\n";
  }
}

void Summary::createNotSupportedSection() {
  mSummary = mSummary %
             "\n- Note: Cameras, lights, and rigging, are currently "
             "only partially supported by the converter. Animations are "
             "currently not supported.\n";
}

void Summary::createUnknownMaterialsSection() {
  auto unknownMaterials = SummarySingleton::getUnknownMaterials();
  if (!unknownMaterials.isEmpty()) {
    mSummary = mSummary % "\n- " % QString::number(unknownMaterials.size()) %
               " partially supported materials\n";
    for (int i = 0; i < unknownMaterials.size(); i++) {
      QString nodeName = unknownMaterials[i].first;
      QString materialName = unknownMaterials[i].second;
      mSummary =
          mSummary % "Node: " % nodeName % ", Material: " % materialName % "\n";
    }
  }
}

void Summary::createMissingTextureFilesSection() {
  auto files = SummarySingleton::getMissingTextureFiles();
  if (files.size() > 0) {
    mSummary = mSummary % "\n- " % QString::number(files.size());
    mSummary =
        mSummary % " texture " % ((files.size() == 1) ? "file" : "files");
    mSummary = mSummary % " not found under " %
               SummarySingleton::getFbxTextureFolderPath() % ":\n";

    auto iter = files.constBegin();
    while (iter != files.constEnd()) {
      mSummary = mSummary % "    " % iter.key() % "\n";
      iter++;
    }
  }
}

void Summary::createReplacedTextureFilesSection() {
  auto files = SummarySingleton::getReplacedTextureFiles();
  if (files.size() > 0) {
    mSummary = mSummary % "\n- " % QString::number(files.size());
    mSummary = mSummary % " texture file(s) were replaced:\n";

    auto iter = files.constBegin();
    while (iter != files.constEnd()) {
      mSummary = mSummary % "    " % iter.key() % " -> " % iter.value() % "\n";
      iter++;
    }

    mSummary = mSummary % "\n";
  }
}

void Summary::createUnsupportedTexturesSection() {
  auto unsupportedTextures = SummarySingleton::getUnsupportedTextures();
  if (!unsupportedTextures.isEmpty()) {
    mSummary = mSummary % "\n- Unsupported textures\n";
    auto iter = unsupportedTextures.constBegin();
    while (iter != unsupportedTextures.constEnd()) {
      QString nodeName = std::get<0>(iter.key());
      QString materialName = std::get<1>(iter.key());
      QString propertyName = std::get<2>(iter.key());
      mSummary = mSummary % "Node: " % nodeName % " (" % materialName;
      mSummary = mSummary % ": " % propertyName % "), Texture name: " %
                 iter.value() % "\n";
      iter++;
    }
  }
}

void Summary::sectionHook(){};

void Summary::footerHook(){};