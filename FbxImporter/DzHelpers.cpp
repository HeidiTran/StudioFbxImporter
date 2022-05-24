#include "DzHelpers.h"

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

void DzHelpers::displayInfoPopup(const QString& title, const QString& text) {
  QMessageBox::information(0, title, text, QMessageBox::Ok);
}

void DzHelpers::displayWarningPopup(const QString& title, const QString& text) {
  QMessageBox::warning(0, title, text, QMessageBox::Ok);
}

void DzHelpers::displayCriticalPopup(const QString& title,
                                     const QString& text) {
  QMessageBox::critical(0, title, text, QMessageBox::Abort);
}

void DzHelpers::logInfo(const QString& text) {
  LOG(INFO) << text.toLocal8Bit().data();
}

void DzHelpers::logInfo(const char* text) { LOG(INFO) << text; }

void DzHelpers::logWarning(const QString& text) {
  LOG(WARNING) << text.toLocal8Bit().data();
}

void DzHelpers::logWarning(const char* text) { LOG(WARNING) << text; }

void DzHelpers::logError(const QString& text) {
  LOG(ERROR) << text.toLocal8Bit().data();
}

void DzHelpers::logError(const char* text) { LOG(ERROR) << text; }

void DzHelpers::logCritical(const QString& text) {
  LOG(FATAL) << text.toLocal8Bit().data();
}

void DzHelpers::logCritical(const char* text) { LOG(FATAL) << text; }

void DzHelpers::logAndDisplayInfo(const QString& title, const QString& text) {
  logInfo(text);
  displayInfoPopup(title, text);
}

void DzHelpers::logAndDisplayWarning(const QString& title,
                                     const QString& text) {
  logWarning(text);
  displayWarningPopup(title, text);
}

void DzHelpers::logAndDisplayCritical(const QString& title,
                                      const QString& text) {
  logCritical(text);
  displayCriticalPopup(title, text);
}

bool DzHelpers::getFbxFilePath(QString& fbxFilePath, const QString& message,
                               const QString& filter) {
  // Check if the main window has been created yet.
  // If it hasn't, alert the user and exit early.
  DzMainWindow* mw = dzApp->getInterface();
  if (!mw) {
    displayWarningPopup("Error", "The main window has not been created yet.");
    return false;
  }

  QFileDialog* fileDlg = new QFileDialog();
  QStringList filters;
  filters.append(filter);

  fileDlg->setFilters(filters);
  fileDlg->setFileMode(
      QFileDialog::ExistingFile);  // Allow only one existing files
  fileDlg->setWindowTitle(message);

  if (fileDlg->exec() != QDialog::Accepted) {
    return true;
  }

  QStringList fl = fileDlg->selectedFiles();
  if (!fl.isEmpty()) {
    fbxFilePath = fl[0];
  }

  return true;
}

bool DzHelpers::getFolderPath(const QString& fbxFilePath, QDir& folderPath,
                              const QString& message) {
  QDir fbxFileDir = fbxFilePath;
  fbxFileDir.cdUp();

  QFileDialog* fileDlg = new QFileDialog();
  fileDlg->setDirectory(fbxFileDir);
  fileDlg->setFileMode(QFileDialog::Directory);
  fileDlg->setOption(QFileDialog::ShowDirsOnly, false);
  fileDlg->setWindowTitle(message);

  if (fileDlg->exec() != QDialog::Accepted) {
    return true;
  }

  QStringList fl = fileDlg->selectedFiles();
  if (!fl.isEmpty()) {
    folderPath = fl[0];
  }

  return true;
}

bool DzHelpers::writeToFile(QString filename, QString content) {
  QFile f(filename);
  if (f.open(QIODevice::ReadWrite)) {
    QTextStream stream(&f);
    stream << content << endl;
    return true;
  }

  return false;
}

DzFigure* DzHelpers::createFigure() {
  DzFigure* dsFigure = new DzFigure();

  DzEnumProperty* followModeControl = NULL;
  if (!QMetaObject::invokeMethod(
          dsFigure, "getFollowModeControl",
          Q_RETURN_ARG(DzEnumProperty*, followModeControl)) ||
      !followModeControl) {
    followModeControl =
        qobject_cast<DzEnumProperty*>(dsFigure->findProperty("Fit to Mode"));
    if (followModeControl) {
      followModeControl->setValue(1);
    }
  }

  return dsFigure;
}

DzNodeList DzHelpers::getChildNodes(DzNode* node, bool isRecursive) {
  DzNodeList childNodes;
  node->getNodeChildren(childNodes, isRecursive);
  return childNodes;
}

/// <summary>
// Get the global position of the node for the current pose.
// If the specified node is not part of the pose or no pose is specified, get
// its global position at the current time.
/// </summary>
FbxAMatrix DzHelpers::GetGlobalPosition(FbxNode* pNode, const FbxTime& pTime,
                                        const FbxPose* pPose,
                                        FbxAMatrix* pParentGlobalPosition) {
  FbxAMatrix lGlobalPosition;
  bool lPositionFound = false;
  if (pPose) {
    int lNodeIndex = pPose->Find(pNode);
    if (lNodeIndex > -1) {
      // The bind pose is always a global matrix.
      // If we have a rest pose, we need to check if it is
      // stored in global or local space.
      if (pPose->IsBindPose() || !pPose->IsLocalMatrix(lNodeIndex)) {
        lGlobalPosition = GetPoseMatrix(pPose, lNodeIndex);
      } else {
        // We have a local matrix, we need to convert it to
        // a global space matrix.
        FbxAMatrix lParentGlobalPosition;
        if (pParentGlobalPosition) {
          lParentGlobalPosition = *pParentGlobalPosition;
        } else {
          if (pNode->GetParent()) {
            lParentGlobalPosition =
                GetGlobalPosition(pNode->GetParent(), pTime, pPose);
          }
        }
        FbxAMatrix lLocalPosition = GetPoseMatrix(pPose, lNodeIndex);
        lGlobalPosition = lParentGlobalPosition * lLocalPosition;
      }
      lPositionFound = true;
    }
  }
  if (!lPositionFound) {
    // There is no pose entry for that node, get the current global position
    // instead. Ideally this would use parent global position and local position
    // to compute the global position. Unfortunately the equation
    //    lGlobalPosition = pParentGlobalPosition * lLocalPosition
    // does not hold when inheritance type is other than "Parent" (RSrs).
    // To compute the parent rotation and scaling is tricky in the RrSs and Rrs
    // cases.
    lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
  }
  return lGlobalPosition;
}

/// <summary>
/// Get the matrix of the given pose
/// </summary>
FbxAMatrix DzHelpers::GetPoseMatrix(const FbxPose* pPose, int pNodeIndex) {
  FbxAMatrix lPoseMatrix;
  FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);
  memcpy((double*)lPoseMatrix, (double*)lMatrix, sizeof(lMatrix.mData));
  return lPoseMatrix;
}
