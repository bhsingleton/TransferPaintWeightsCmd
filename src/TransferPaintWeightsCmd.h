#ifndef _TRANSFER_PAINT_WEIGHTS_CMD
#define _TRANSFER_PAINT_WEIGHTS_CMD
//
// File: TransferPaintWeightsCmd.h
//
// MEL Command: TransferPaintWeights
//
// Author: Benjamin H. Singleton
//

#include <maya/MPxCommand.h>

#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>

#include <maya/MItMeshPolygon.h>

#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSyntax.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MColor.h>
#include <maya/MColorArray.h>
#include <maya/MStatus.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>
#include <maya/MStreamUtils.h>

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

#define COMMAND_NAME "transferPaintWeights"
#define COLOR_SET_NAME "paintWeightsColorSet1"
#define COLOR_RAMP "0.5,0.5,0.5,0.5,1,1,1,1,1,1,0,0,0,0,1"
#define CHUNK_SIZE 5
#define RAMP_DELIMITER ','
#define MIN_COLOR MColor(0.0f, 0.0f, 0.0f)
#define MAX_COLOR MColor(1.0f, 1.0f, 1.0f)


class TransferPaintWeightsCmd : public MPxCommand 
{

public:

						TransferPaintWeightsCmd();
	virtual				~TransferPaintWeightsCmd();

	virtual MStatus		doIt(const MArgList &args);
	virtual	MStatus		redoIt();
	virtual	bool		isUndoable() const;
	
	static	int			clamp(int value, int min, int max);
	static	MIntArray	range(unsigned int start, unsigned int end, unsigned int step);

	static	MStatus		markPlugDirty(const MPlug& plug);
	static	bool		hasColorSet(const MDagPath &dagPath, const MString colorSetName);
	static	MIntArray	createColorIds(const MDagPath &mesh, const MDoubleArray &weights, const MColorArray &colors);
	static	MStatus		createGradient(const MString colorRamp, MColorArray &gradient);
	static	MStatus		applyPaintWeights(const MDagPath &mesh, const MDoubleArray &weights, const MColorArray &gradient, MString colorSetName);

	static  void*		creator();

	static	MSyntax		newSyntax();
	virtual	bool		hasSyntax();

protected:

			MSelectionList	selection;
			MString			colorSetName;
			MDoubleArray	weights;

			MString			colorRamp;
			MColor			rampMinColor;
			MColor			rampMaxColor;
			MColorArray		gradient;

};


struct ColorSlot 
{

	MColor		color;
	float		position;

public:

				ColorSlot() {};
				ColorSlot(float r, float g, float b, float position) : color(r, g, b, 1.0), position(position) {};

	virtual		~ColorSlot() {};

};


struct less_than_key 
{

	inline bool operator() (const ColorSlot& slot1, const ColorSlot& slot2) { return (slot1.position < slot2.position); }

};

#endif