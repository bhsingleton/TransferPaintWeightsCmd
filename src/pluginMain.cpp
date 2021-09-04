//
// File: pluginMain.cpp
//
// Author: Benjamin H. Singleton
//

#include "TransferPaintWeightsCmd.h"
#include <maya/MFnPlugin.h>


MStatus initializePlugin(MObject obj) 
{

	MStatus   status;

	MFnPlugin plugin(obj, "Ben Singleton", "2017", "Any");
	status = plugin.registerCommand(COMMAND_NAME, TransferPaintWeightsCmd::creator, TransferPaintWeightsCmd::newSyntax);

	if (!status) 
	{

		status.perror("registerNode");
		return status;

	}

	return status;

}

MStatus uninitializePlugin(MObject obj) 
{

	MStatus   status;

	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand(COMMAND_NAME);

	if (!status) 
	{

		status.perror("deregisterNode");
		return status;

	}

	return status;

}
