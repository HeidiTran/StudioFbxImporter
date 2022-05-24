#pragma once

#include "QtGui"
#include "dzapp.h"
#include "dzenumproperty.h"
#include "dzfigure.h"
#include "dzmainwindow.h"
#include "dzproperty.h"
#include "fbxsdk.h"

namespace DzHelpers {
void displayInfoPopup(const QString& title, const QString& text);
void displayWarningPopup(const QString& title, const QString& text);
void displayCriticalPopup(const QString& title, const QString& text);

void logInfo(const QString& text);
void logInfo(const char* text);
void logWarning(const QString& text);
void logWarning(const char* text);
void logError(const QString& text);
void logError(const char* text);
void logCritical(const QString& text);
void logCritical(const char* text);

void logAndDisplayInfo(const QString& title, const QString& text);
void logAndDisplayWarning(const QString& title, const QString& text);
void logAndDisplayCritical(const QString& title, const QString& text);

bool getFbxFilePath(QString& fbxFilePath, const QString& message,
                    const QString& filter);
bool getFolderPath(const QString& fbxFilePath, QDir& folderPath,
                   const QString& message);
bool writeToFile(QString filename, QString content);

DzFigure* createFigure();
DzNodeList getChildNodes(DzNode* node, bool isRecursive = false);

FbxAMatrix GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime,
                             const FbxPose* pPose = NULL,
                             FbxAMatrix* pParentGlobalPosition = NULL);

FbxAMatrix GetPoseMatrix(const FbxPose* pPose, int pNodeIndex);
}  // namespace DzHelpers
