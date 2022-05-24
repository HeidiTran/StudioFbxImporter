#include "ShortSummary.h"

ShortSummary::ShortSummary(QString longSummaryPath) {
  mLongSummaryPath = longSummaryPath;
}

ShortSummary::~ShortSummary() {}

void ShortSummary::createUnknownMaterialsSection() {
  auto unknownMaterials = SummarySingleton::getUnknownMaterials();
  if (unknownMaterials.size() > 0) {
    mSummary = mSummary % "\n- " % QString::number(unknownMaterials.size()) %
               " partially supported materials found:\n";
    for (int i = 0, n = qMin(unknownMaterials.size(), DISPLAY_LIMIT); i < n;
         i++) {
      mSummary = mSummary % "    " % unknownMaterials[i].second % "\n";
    }

    if (unknownMaterials.size() > DISPLAY_LIMIT) {
      mSummary = mSummary % "    ...\n";
    }

    mSummary = mSummary % "\n";
  }
}

void ShortSummary::createMissingTextureFilesSection() {
  auto files = SummarySingleton::getMissingTextureFiles();
  if (files.size() > 0) {
    mSummary = mSummary % "\n- " % QString::number(files.size());
    mSummary =
        mSummary % " texture " % ((files.size() == 1) ? "file" : "files");
    mSummary = mSummary % " not found under " %
               SummarySingleton::getFbxTextureFolderPath() % ":\n";

    auto iter = files.begin();
    for (int i = 0, n = qMin(files.size(), DISPLAY_LIMIT); i < n; i++) {
      mSummary = mSummary % "    " % iter.key() % "\n";
      iter++;
    }

    if (files.size() > DISPLAY_LIMIT) {
      mSummary = mSummary % "    ...\n";
    }
  }
}

void ShortSummary::createReplacedTextureFilesSection() {
  auto files = SummarySingleton::getReplacedTextureFiles();
  if (files.size() > 0) {
    mSummary = mSummary % "\n- " % QString::number(files.size());
    mSummary = mSummary % " texture file(s) were replaced:\n";
    auto iter = files.constBegin();
    for (int i = 0, n = qMin(files.size(), DISPLAY_LIMIT); i < n; i++) {
      mSummary = mSummary % "    " % iter.key() % " -> " % iter.value() % "\n";
      iter++;
    }

    if (files.size() > DISPLAY_LIMIT) {
      mSummary = mSummary % "    ...\n";
    }

    mSummary = mSummary % "\n";
  }
}

void ShortSummary::createUnsupportedTexturesSection() {
  auto unsupportedTextures =
      SummarySingleton::getUnsupportedTextures().values();
  QSet<QString> textureNames;
  for (int i = 0; i < unsupportedTextures.size(); i++) {
    textureNames.insert(unsupportedTextures.at(i));
  }

  if (!unsupportedTextures.isEmpty()) {
    mSummary = mSummary % "\n- " % QString::number(textureNames.size()) %
               " unsupported texture(s)\n";
  }
}

void ShortSummary::footerHook() {
  mSummary = mSummary % "\nFor more details, open " % mLongSummaryPath % "\n";
}
