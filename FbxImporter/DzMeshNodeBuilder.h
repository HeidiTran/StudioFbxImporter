#pragma once

#include <array>

#include "AbstractNodeBuilder.h"
#include "DzMaterialFactory.h"
#include "MTLParser.h"
#include "dzgeometry.h"

class DzMeshNodeBuilder : public AbstractNodeBuilder {
  struct UVInformation {
    QString uvSetName = "";
    QList<std::array<int, 4>> uvVertexIndices;
    QList<std::array<float, 2>> UVs;
  };

 public:
  DzMeshNodeBuilder(FbxNode* root, QDir fbxTextureFolderPath,
                    const QMap<QString, MTLMap>& materialNameToMTLMap,
                    QMap<FbxMesh*, DzFigure*>* fbxSkinToDzFigurePtr);
  ~DzMeshNodeBuilder();

  void buildNode();

 private:
  QDir mFbxTextureFolderPath;
  QMap<QString, MTLMap> mMaterialNameToMTLMap;
  QMap<FbxMesh*, DzFigure*>* mfbxMeshToDzFigurePtr;

  QList<DzMaterial*> createMaterials();
  QList<std::array<int, 3>> loadNormalInformation(FbxMesh* mesh);
  UVInformation loadUVInformation(FbxMesh* mesh);
  void setFacetUVIndices(DzFacet& facet,
                         const QList<std::array<int, 4Ui64>>& UVVertexIndices,
                         const int polygonIdx);
  void setFacetVertIndices(DzFacet& facet, int* facetVertices, bool isTriangle);
  void setFacetMeshUVs(DzFacetMesh* facetMesh,
                       const DzMeshNodeBuilder::UVInformation& uvInfo);
  void setFacetNormalIndices(DzFacet& facet,
                             const QList<std::array<int, 3Ui64>>& dzNormals,
                             const int polygonIdx);
  void setVertexWeights(FbxMesh* fbxMesh, DzFacetShape* facetShape,
                        DzFacetMesh* facetMesh);
  void enableSubdivision(DzFacetMesh* facetMesh, DzFacetShape* facetShape);
};
