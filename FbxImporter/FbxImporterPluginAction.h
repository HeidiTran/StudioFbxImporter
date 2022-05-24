#pragma once

#include "FbxToDazImporter.h"

class DzFbxImporterAction : public DzAction {
  Q_OBJECT

 public:
  DzFbxImporterAction();

  // These get called by DzActionMgr during initialization.
  virtual QString getActionGroup() const { return tr("Import FBX"); }
  virtual QString getDefaultMenuPath() const { return tr("&Import FBX"); }

 protected:
  // We need to re-implement this virtual function to perform our Fbx Importer
  // Action. This gets called whenever the user clicks on our action item
  virtual void executeAction();
};