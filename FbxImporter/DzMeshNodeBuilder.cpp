#include "DzMeshNodeBuilder.h"

DzMeshNodeBuilder::DzMeshNodeBuilder(
    FbxNode* root, QDir fbxTextureFolderPath,
    const QMap<QString, MTLMap>& materialNameToMTLMap,
    QMap<FbxMesh*, DzFigure*>* fbxSkinToDzFigurePtr) {
  mRoot = root;
  mFbxTextureFolderPath = fbxTextureFolderPath;
  mMaterialNameToMTLMap = materialNameToMTLMap;
  mfbxMeshToDzFigurePtr = fbxSkinToDzFigurePtr;
}

void DzMeshNodeBuilder::buildNode() {
  FbxMesh* fbxMesh = mRoot->GetMesh();
  int fbxVerticesCnt = fbxMesh->GetControlPointsCount();
  FbxVector4* fbxVertices = fbxMesh->GetControlPoints();

  // Validate mesh node data.
  if (fbxVerticesCnt == 0 || fbxVertices == nullptr) {
    throw std::logic_error("List of vertices is empty.");
  }

  // Create new node
  mDzNode = new DzNode();

  // If this mesh has a skin -> it's a figure
  int skinCnt = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
  if (skinCnt > 0) {
    DzFigure* dzFigure = DzHelpers::createFigure();
    mDzNode = dzFigure;
    mfbxMeshToDzFigurePtr->insert(fbxMesh, dzFigure);
  }

  mDzNode->setName(mRoot->GetName());
  mDzNode->setLabel(mDzNode->getName());

  DzObject* dzObject = new DzObject();
  DzFacetShape* dzFacetShape = new DzFacetShape();
  DzFacetMesh* dzFacetMesh = new DzFacetMesh();

  //// ------------ Begin the edits ------------ ////
  dzFacetMesh->beginEdit();

  // Add all vertices to the mesh
  // `setVertexArray` Resizes the vertex array to numVerts items and returns a
  // pointer to the array
  DzPnt3* dzVertices = dzFacetMesh->setVertexArray(fbxVerticesCnt);
  for (int i = 0; i < fbxVerticesCnt; i++) {
    dzVertices[i][0] = fbxVertices[i][0];
    dzVertices[i][1] = fbxVertices[i][1];
    dzVertices[i][2] = fbxVertices[i][2];
  }

  UVInformation dzUvInfo = loadUVInformation(fbxMesh);
  QList<std::array<int, 4>> UVVertexIndices = dzUvInfo.uvVertexIndices;
  setFacetMeshUVs(dzFacetMesh, dzUvInfo);

  QList<std::array<int, 3>> dzNormals = loadNormalInformation(fbxMesh);

  setVertexWeights(fbxMesh, dzFacetShape, dzFacetMesh);

  // Add material information to the mesh and the shape
  QList<DzMaterial*> dzMaterials = createMaterials();
  for (int i = 0; i < dzMaterials.size(); ++i) {
    DzMaterial* mat = dzMaterials.at(i);
    dzFacetShape->addMaterial(mat);
    dzFacetMesh->activateMaterial(mat->getName());
  }

  // check whether the material maps with only one mesh
  bool matsAllSame = true;
  for (int i = 0; i < fbxMesh->GetElementMaterialCount(); i++) {
    FbxGeometryElementMaterial* lMaterialElement =
        fbxMesh->GetElementMaterial(i);
    if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon) {
      matsAllSame = false;
      break;
    }
  }

  if (matsAllSame) {
    for (int i = 0; i < fbxMesh->GetElementMaterialCount(); i++) {
      FbxGeometryElementMaterial* lMaterialElement =
          fbxMesh->GetElementMaterial(i);
      if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame) {
        FbxSurfaceMaterial* lMaterial = fbxMesh->GetNode()->GetMaterial(
            lMaterialElement->GetIndexArray().GetAt(0));
        Q_UNUSED(lMaterial);

        const int lMatId = lMaterialElement->GetIndexArray().GetAt(0);
        if (lMatId >= 0) {
          dzFacetMesh->activateMaterial(lMatId);
          break;
        }
      }
    }
  }

  int* polygonVertices = fbxMesh->GetPolygonVertices();
  const int numPolygons = fbxMesh->GetPolygonCount();
  FbxGeometryElementPolygonGroup* polygonGroup =
      fbxMesh->GetElementPolygonGroup(0);
  dzFacetMesh->preSizeFacets(numPolygons);
  int curGroupId = -1;

  for (int i = 0; i < numPolygons; i++) {
    if (!matsAllSame) {
      for (int j = 0; j < fbxMesh->GetElementMaterialCount(); j++) {
        FbxGeometryElementMaterial* lMaterialElement =
            fbxMesh->GetElementMaterial(j);
        FbxSurfaceMaterial* lMaterial = fbxMesh->GetNode()->GetMaterial(
            lMaterialElement->GetIndexArray().GetAt(i));
        const int lMatId = lMaterialElement->GetIndexArray().GetAt(i);

        if (lMatId >= 0) {
          dzFacetMesh->activateMaterial(lMatId);
          break;
        }
      }
    }

    if (polygonGroup) {
      const int groupId = polygonGroup->GetIndexArray().GetAt(i);
      if (groupId != curGroupId) {
        curGroupId = groupId;
        dzFacetMesh->activateFaceGroup("fbx_group_" % QString::number(groupId));
      }
    }

    // Create the faces
    DzFacet facet;
    int* facetVertices = &(polygonVertices[fbxMesh->GetPolygonVertexIndex(i)]);
    setFacetVertIndices(facet, facetVertices, fbxMesh->GetPolygonSize(i) == 3);
    setFacetUVIndices(facet, UVVertexIndices, i);
    setFacetNormalIndices(facet, dzNormals, i);
    QMetaObject::invokeMethod(dzFacetMesh, "addFacet",
                              Q_ARG(const DzFacet&, facet));
  }

  //// ------------ End the edits ------------ ////
  dzFacetMesh->finishEdit();

  setGeometricTransformation(dzFacetMesh);
  setNodeBoundingBox(mDzNode);
  setNodePresentation(mDzNode, "Prop");
  setNodeProperties(mDzNode);

  dzFacetShape->setFacetMesh(dzFacetMesh);
  dzObject->addShape(dzFacetShape);
  mDzNode->setObject(dzObject);
}

void DzMeshNodeBuilder::setFacetNormalIndices(
    DzFacet& facet, const QList<std::array<int, 3Ui64>>& dzNormals,
    int polygonIdx) {
  if (dzNormals.size() > 0) {
    facet.m_normIdx[0] = dzNormals[polygonIdx][0];
    facet.m_normIdx[1] = dzNormals[polygonIdx][1];
    facet.m_normIdx[2] = dzNormals[polygonIdx][2];
  }
}

void DzMeshNodeBuilder::setVertexWeights(FbxMesh* fbxMesh,
                                         DzFacetShape* facetShape,
                                         DzFacetMesh* facetMesh) {
  for (int i = 0; i < fbxMesh->GetElementVertexCreaseCount(); i++) {
    FbxGeometryElementCrease* element = fbxMesh->GetElementVertexCrease(i);
    const int num = element->GetDirectArray().GetCount();

    for (int j = 0; j < num; j++) {
      double weight = element->GetDirectArray().GetAt(j);
      if (weight > 0) {
        DzHelpers::logWarning("Has vetex weights");
        enableSubdivision(facetMesh, facetShape);
        QMetaObject::invokeMethod(facetMesh, "setVertexWeight", Q_ARG(int, j),
                                  Q_ARG(int, weight));
      }
    }

    break;
  }
}

void DzMeshNodeBuilder::enableSubdivision(DzFacetMesh* facetMesh,
                                          DzFacetShape* facetShape) {
  facetMesh->enableSubDivision(true);

  if (DzEnumProperty* lodControl = facetShape->getLODControl()) {
    lodControl->setValue(lodControl->getNumItems() - 1);  // set to high res
    lodControl->setDefaultValue(lodControl->getNumItems() -
                                1);  // set to high res
  }
}

void DzMeshNodeBuilder::setFacetVertIndices(DzFacet& facet, int* facetVertices,
                                            bool isTriangle) {
  // Since we triangulated the scene, a polygon can only be a triangle or a
  // quadrilateral. If it's a triangle, the last value in the vector is -1
  facet.m_vertIdx[0] = facetVertices[0];
  facet.m_vertIdx[1] = facetVertices[1];
  facet.m_vertIdx[2] = facetVertices[2];
  facet.m_vertIdx[3] = isTriangle ? -1 : facetVertices[3];
}

void DzMeshNodeBuilder::setFacetUVIndices(
    DzFacet& facet, const QList<std::array<int, 4Ui64>>& UVVertexIndices,
    const int polygonIdx) {
  if (UVVertexIndices.size() == 0) {
    facet.m_uvwIdx[0] = 0;
    facet.m_uvwIdx[1] = 0;
    facet.m_uvwIdx[2] = 0;
    facet.m_uvwIdx[3] = 0;
  } else {
    facet.m_uvwIdx[0] = UVVertexIndices[polygonIdx][0];
    facet.m_uvwIdx[1] = UVVertexIndices[polygonIdx][1];
    facet.m_uvwIdx[2] = UVVertexIndices[polygonIdx][2];
    facet.m_uvwIdx[3] = UVVertexIndices[polygonIdx][3];
  }
}

/// <summary>
/// Get the mesh's UV map and populate the UVs info
/// This MUST be done before creating the faces since the face will reference
/// the UV's index
/// </summary>
void DzMeshNodeBuilder::setFacetMeshUVs(
    DzFacetMesh* facetMesh, const DzMeshNodeBuilder::UVInformation& uvInfo) {
  QList<std::array<float, 2>> UVs = uvInfo.UVs;
  if (UVs.length() != 0) {
    DzMap* map = facetMesh->getUVs();
    map->setName(uvInfo.uvSetName);

    for (int i = 0; i < UVs.length(); i++) {
      DzPnt2 pUVs = {UVs[i][0], UVs[i][1]};
      map->appendPnt2Value(pUVs);
    }
  }
}

QList<DzMaterial*> DzMeshNodeBuilder::createMaterials() {
  DzMaterialList dzMaterials;
  if (mRoot == nullptr) {
    return dzMaterials;
  }

  DzMaterialFactory* dzMaterialFactory =
      new DzMaterialFactory(mFbxTextureFolderPath, mMaterialNameToMTLMap);

  int materialCount = mRoot->GetMaterialCount();
  if (materialCount == 0) {
    SummarySingleton::addNodeNameMaterial(mRoot->GetName(),
                                          "DazDefaultMaterial");
    DzMaterial* dzMaterial = dzMaterialFactory->createDefaultMaterial();
    dzMaterials.append(dzMaterial);
  }

  for (int i = 0; i < materialCount; i++) {
    FbxSurfaceMaterial* fbxMaterial = mRoot->GetMaterial(i);
    DzMaterial* dzMaterial =
        dzMaterialFactory->createMaterial(mRoot->GetName(), fbxMaterial);
    dzMaterials.append(dzMaterial);
  }

  return dzMaterials;
}

// Source:
// https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_cpp_ref_normals_2main_8cxx_example_html
QList<std::array<int, 3>> DzMeshNodeBuilder::loadNormalInformation(
    FbxMesh* mesh) {
  QList<std::array<int, 3>> retNormals;

  // get the normal element
  FbxGeometryElementNormal* lNormalElement = mesh->GetElementNormal();
  if (lNormalElement) {
    // mapping mode is by control points. The mesh should be smooth and soft.
    // we can get normals by retrieving each control point
    if (lNormalElement->GetMappingMode() ==
        FbxGeometryElement::eByControlPoint) {
      // TODO: Support this mapping mode

      SummarySingleton::increaseNormalMappedByVertexCnt();
      return retNormals;

      // Let's get normals of each vertex, since the mapping mode of normal
      // element is by control point
      for (int lVertexIndex = 0; lVertexIndex < mesh->GetControlPointsCount();
           lVertexIndex++) {
        int lNormalIndex = 0;
        // reference mode is direct, the normal index is same as vertex index.
        // get normals by the index of control vertex
        if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
          lNormalIndex = lVertexIndex;
        // reference mode is index-to-direct, get normals by the index-to-direct
        if (lNormalElement->GetReferenceMode() ==
            FbxGeometryElement::eIndexToDirect)
          lNormalIndex = lNormalElement->GetIndexArray().GetAt(lVertexIndex);
        // Got normals of each vertex.
        FbxVector4 lNormal =
            lNormalElement->GetDirectArray().GetAt(lNormalIndex);
        // if (gVerbose) FBXSDK_printf("normals for vertex[%d]: %f %f %f %f \n",
        // lVertexIndex, lNormal[0], lNormal[1], lNormal[2], lNormal[3]); add
        // your custom code here, to output normals or get them into a list,
        // such as KArrayTemplate<FbxVector4> . . .
      }  // end for lVertexIndex
    }    // end eByControlPoint
    // mapping mode is by polygon-vertex.
    // we can get normals by retrieving polygon-vertex.
    else if (lNormalElement->GetMappingMode() ==
             FbxGeometryElement::eByPolygonVertex) {
      int lIndexByPolygonVertex = 0;
      // Let's get normals of each polygon, since the mapping mode of normal
      // element is by polygon-vertex.
      for (int lPolygonIndex = 0; lPolygonIndex < mesh->GetPolygonCount();
           lPolygonIndex++) {
        // get polygon size, you know how many vertices in current polygon.
        int lPolygonSize = mesh->GetPolygonSize(lPolygonIndex);
        // retrieve each vertex of current polygon.
        for (int i = 0; i < lPolygonSize; i++) {
          int lNormalIndex = 0;
          // reference mode is direct, the normal index is same as
          // lIndexByPolygonVertex.
          if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect)
            lNormalIndex = lIndexByPolygonVertex;
          // reference mode is index-to-direct, get normals by the
          // index-to-direct
          if (lNormalElement->GetReferenceMode() ==
              FbxGeometryElement::eIndexToDirect)
            lNormalIndex =
                lNormalElement->GetIndexArray().GetAt(lIndexByPolygonVertex);
          // Got normals of each polygon-vertex.
          FbxVector4 lNormal =
              lNormalElement->GetDirectArray().GetAt(lNormalIndex);
          std::array<int, 3> normal = {lNormal[0], lNormal[1], lNormal[2]};
          retNormals.append(normal);
          lIndexByPolygonVertex++;
        }  // end for i //lPolygonSize
      }    // end for lPolygonIndex //PolygonCount
    }      // end eByPolygonVertex
  }        // end if lNormalElement

  return retNormals;
}

// Source:
// https://help.autodesk.com/view/FBX/2020/ENU/?guid=FBX_Developer_Help_cpp_ref_u_v_sample_2main_8cxx_example_html
DzMeshNodeBuilder::UVInformation DzMeshNodeBuilder::loadUVInformation(
    FbxMesh* mesh) {
  UVInformation uvInfo;

  FbxStringList lUVSetNameList;
  mesh->GetUVSetNames(lUVSetNameList);

  for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount();
       lUVSetIndex++) {
    const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);

    uvInfo.uvSetName = lUVSetName;

    const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetName);
    if (!lUVElement) {
      continue;
    }

    for (int i = 0; i < lUVElement->GetDirectArray().GetCount(); i++) {
      FbxVector2 uvVector = lUVElement->GetDirectArray().GetAt(i);
      std::array<float, 2> temp = {uvVector[0], uvVector[1]};
      uvInfo.UVs.push_back(temp);
    }

    // only support mapping mode eByPolygonVertex and eByControlPoint
    if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
        lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint) {
      DzHelpers::logAndDisplayCritical(
          "Unsupported Mapping Mode for Mesh UV Information",
          "The FBX importer only support the eByPolygonVertex and "
          "eByControlPoint mapping mode.");
    }

    // index array, where holds the index referenced to the uv data
    const bool lUseIndex =
        lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
    const int lIndexCount =
        (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

    // iterating through the data by polygon
    const int lPolyCount = mesh->GetPolygonCount();
    if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
      for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex) {
        std::array<int, 4> uvwIdx;
        // build the max index array that we need to pass into MakePoly
        const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
        for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex) {
          // get the index of the current vertex in control points array
          int lPolyVertIndex = mesh->GetPolygonVertex(lPolyIndex, lVertIndex);

          // the UV index depends on the reference mode
          int lUVIndex = lUseIndex
                             ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex)
                             : lPolyVertIndex;
          uvwIdx[lVertIndex] = lUVIndex;
        }
        uvInfo.uvVertexIndices.append(uvwIdx);
      }
    } else if (lUVElement->GetMappingMode() ==
               FbxGeometryElement::eByPolygonVertex) {
      int lPolyIndexCounter = 0;
      for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex) {
        std::array<int, 4> uvwIdx;

        // build the max index array that we need to pass into MakePoly
        const int lPolySize = mesh->GetPolygonSize(lPolyIndex);
        for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex) {
          if (lPolyIndexCounter < lIndexCount) {
            int lUVIndex =
                lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter)
                          : lPolyIndexCounter;

            uvwIdx[lVertIndex] = lUVIndex;
            lPolyIndexCounter++;
          }
        }

        uvInfo.uvVertexIndices.append(uvwIdx);
      }
    }
  }

  return uvInfo;
}

DzMeshNodeBuilder::~DzMeshNodeBuilder() { delete mDzNode; }
