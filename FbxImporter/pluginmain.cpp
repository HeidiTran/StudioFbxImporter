// pluginmain.cpp : Defines the entry point for the DLL application.

#include "FbxImporterPluginAction.h"
#include "dzapp.h"
#include "dzplugin.h"
#include "version.h"

DZ_PLUGIN_DEFINITION("FBX Importer (Senior Design Project)");
DZ_PLUGIN_AUTHOR("Drexel FbxImporter Senior Design Group");
DZ_PLUGIN_VERSION(PLUGIN_MAJOR, PLUGIN_MINOR, PLUGIN_REV, PLUGIN_BUILD);
DZ_PLUGIN_DESCRIPTION("Import the Autodesk FBX format.");

/**
	This macro adds our class to the objects exported by the plugin, and
	specifies the GUID (Globally Unique Identifier) that makes this class unique
	from any other class that is available from Daz Studio or one of it's plug-ins.
	Make sure that you generate a new GUID for every class that you export from your
	plug-in. See the 'ClassIDs' page in the Daz Studio SDK documentation for more 
	information.
**/
DZ_PLUGIN_CLASS_GUID(DzFbxImporterAction,
                     A5726E19 - 40FA - 495b - B11D -
                         80C03BF1A80Z);  // <UNIQUE UUID!!! HERE>
