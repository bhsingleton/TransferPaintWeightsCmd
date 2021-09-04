//
// File: TransferPaintWeightsCmd.cpp
//
// MEL Command: TransferPaintWeights
//
// Author: Benjamin H. Singleton
//

#include "TransferPaintWeightsCmd.h"


TransferPaintWeightsCmd::TransferPaintWeightsCmd() {}
TransferPaintWeightsCmd::~TransferPaintWeightsCmd() {}


MStatus TransferPaintWeightsCmd::doIt(const MArgList &args)
/**
This method should perform a command by setting up internal class data and then calling the redoIt method.
The actual action performed by the command should be done in the redoIt method. This is a pure virtual method, and must be overridden in derived classes.

@param args: List of arguments passed to the command.
@return Status code.
*/
{

	MStatus status;

	// Reset read buffer
	//
	std::cout.set_rdbuf(MStreamUtils::stdOutStream().rdbuf());
	std::cerr.set_rdbuf(MStreamUtils::stdErrorStream().rdbuf());

	// Initialize argument parser
	//
	MSyntax syntax = TransferPaintWeightsCmd::syntax();

	MArgDatabase argData(syntax, args, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Pass first two objects to selection list
	//
	status = argData.getObjects(this->selection);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Check for "-colorSetName" flag
	//
	bool isFlagSet = argData.isFlagSet("csn", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (isFlagSet) 
	{

		this->colorSetName = argData.flagArgumentString("csn", 0, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		//std::cout << "TransferPaintWeightsCmd::doIt(): Using \"" << this->colorSetName.asChar() << "\" as default color set." << std::endl;

	}
	else 
	{

		this->colorSetName = MString(COLOR_SET_NAME);

	}

	// Check for "-colorRamp" flag
	//
	isFlagSet = argData.isFlagSet("cr", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	if (isFlagSet) 
	{

		this->colorRamp = argData.flagArgumentString("cr", 0, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		//std::cout << "TransferPaintWeightsCmd::doIt(): Using \"" << this->colorRamp.asChar() << "\" as default color ramp." << std::endl;

	}
	else 
	{

		this->colorRamp = MString(COLOR_RAMP);

	}

	// Define color gradient from ramp
	//
	status = TransferPaintWeightsCmd::createGradient(this->colorRamp, this->gradient);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Check for "-rampMinColor" flag
	//
	unsigned int flagUses = argData.numberOfFlagUses("rmc");

	if (flagUses == 3) 
	{

		// Iterate through flags
		//
		MArgList multiArgs;
		MColor minColor;
		double value;

		for (unsigned int i = 0; i < flagUses; i++) 
		{

			status = argData.getFlagArgumentList("rmc", i, multiArgs);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			value = multiArgs.asDouble(i, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			minColor[i] = static_cast<float>(value);

		}

		this->rampMinColor = minColor;

	}
	else 
	{

		this->rampMinColor = MIN_COLOR;

	}

	// Check for "-rampMaxColor" flag
	//
	flagUses = argData.numberOfFlagUses("rxc");

	if (flagUses == 3) 
	{

		// Iterate through flags
		//
		MArgList multiArgs;
		MColor maxColor;
		double value;

		for (unsigned int i = 0; i < flagUses; i++) {

			status = argData.getFlagArgumentList("rxc", i, multiArgs);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			value = multiArgs.asDouble(i, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			maxColor[i] = static_cast<float>(value);

		}

		this->rampMaxColor = maxColor;

	}
	else 
	{

		this->rampMaxColor = MAX_COLOR;

	}

	// Incorporate min/ax colors into gradient
	//
	this->gradient[0] = this->rampMinColor;
	this->gradient[100] = this->rampMaxColor;

	// Call redo it function
	//
	return TransferPaintWeightsCmd::redoIt();

};


MStatus TransferPaintWeightsCmd::redoIt() 
/**
This method should do the actual work of the command based on the internal class data only.
Internal class data should be set in the doIt method.

@return: Status code.
*/
{

	MStatus status;

	// Get ".paintWeights" plug from selection list
	//
	MPlug plug;
	status = this->selection.getPlug(0, plug);
	
	if (!status) {

		status.perror("First argument is not a valid plug!");
		return status;

	}

	// Get double array from plug
	//
	MObject data = plug.asMObject(MDGContext::fsNormal, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MFnDoubleArrayData fnDoubleArrayData(data, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	this->weights = fnDoubleArrayData.array(&status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Get poly modifier from selection list
	//
	MDagPath intermediate;
	status = this->selection.getDagPath(1, intermediate);

	if (!status) {

		status.perror("Second argument is not a valid dag node!");
		return status;

	}

	// Assign weights to intermediate object
	//
	status = TransferPaintWeightsCmd::applyPaintWeights(intermediate, this->weights, this->gradient, this->colorSetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;

};


bool TransferPaintWeightsCmd::isUndoable() const 
/**
Evaluates if this command is undoable.
This command was original developed for Vertex Blender so we want the tool to handle disposing of the color set.

@return: Boolean.
*/
{

	return false;

};


bool TransferPaintWeightsCmd::hasColorSet(const MDagPath &dagPath, const MString colorSetName) 
{

	MStatus status;

	// Initialize function set
	//
	MFnMesh fnMesh(dagPath, &status);
	CHECK_MSTATUS(status);

	// Get all color set names
	//
	MStringArray colorSetNames;

	status = fnMesh.getColorSetNames(colorSetNames);
	CHECK_MSTATUS(status);

	// Iterate through array
	//
	unsigned int numColorSets = colorSetNames.length();

	for (unsigned int i = 0; i < numColorSets; i++) 
	{

		if (colorSetNames[i] == colorSetName) 
		{

			return true;

		}

	}

	return false;

};


MIntArray TransferPaintWeightsCmd::createColorIds(const MDagPath &mesh, const MDoubleArray &weights, const MColorArray &colors) 
/**
Returns a list of face-vertex colour IDs based on the supplied weights and colors.

@param mesh: The mesh to generate for.
@param weights: The normalized weights to calculate a colour from.
@param colors: The array colours to generate indices for.
@return: The list of colour indices.
*/
{

	MStatus status;

	// Define color id range
	//
	unsigned int min = 0;

	unsigned int numColors = colors.length();
	unsigned int max = numColors - 1;

	// Get number of face vertices
	//
	MFnMesh fnMesh(mesh, &status);
	CHECK_MSTATUS(status);

	unsigned int numFaceVertices = fnMesh.numFaceVertices(&status);
	CHECK_MSTATUS(status);

	// Initialize polygon iterator
	//
	MItMeshPolygon iterPolygon(mesh);
	
	MIntArray colorIds(numFaceVertices, 0);
	MIntArray vertices;

	double value;
	int colorIndex;

	unsigned int numVertices;
	unsigned int insertAt = 0;

	for (iterPolygon.reset(); !iterPolygon.isDone(); iterPolygon.next()) 
	{

		// Get face vertices
		//
		status = iterPolygon.getVertices(vertices);
		CHECK_MSTATUS(status);

		numVertices = vertices.length();

		for (unsigned int i = 0; i < numVertices; i++) 
		{

			// Scale and clamp value to index range
			//
			value = ceil(weights[vertices[i]] * max);
			colorIndex = TransferPaintWeightsCmd::clampValue(static_cast<int>(value), min, max);

			colorIds[insertAt] = colorIndex;

			// Incrememnt insertion index
			//
			insertAt += 1;

		}

	}
	
	return colorIds;

};


MStatus TransferPaintWeightsCmd::getWeights(const MObject &skinCluster, unsigned int influenceId, const MIntArray &vertices, MDoubleArray &weights)
/**
Collects all of the weight values for the given influence ID.

@param skinCluster: The skin cluster to sample from.
@param influenceId: The influence ID to sample from.
@param vertices: The vertex indices to sample from.
@param weights: The passed array to populate.
@return: Status code..
*/
{

	MStatus status;

	// Initialize function set
	//
	MFnDependencyNode fnDepNode(skinCluster, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Get ".weightList" plug
	//
	MPlug plug = fnDepNode.findPlug("weightList", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	MObject attribute = fnDepNode.attribute("weights", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Resize array to accomodate vertex quantity
	//
	unsigned int numVertices = vertices.length();

	status = weights.setLength(numVertices);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Iterate through all vertices
	//
	MPlug child, element;

	unsigned int numChildElements;
	unsigned int elementIndex;

	for (unsigned int i = 0; i < numVertices; i++) 
	{

		// Jump to next index
		//
		status = plug.selectAncestorLogicalIndex(vertices[i]);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// Set default value
		//
		weights[i] = 0.0;

		// Get ".weight" child plug
		//
		child = plug.child(attribute, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		// Iterate through child elements
		// These are sparse indices so get existing indices!
		//
		numChildElements = child.numElements(&status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

		for (unsigned int j = 0; j < numChildElements; j++) 
		{

			// Check if influence id matches
			//
			element = child.elementByPhysicalIndex(j, &status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			elementIndex = element.logicalIndex(&status);
			CHECK_MSTATUS_AND_RETURN_IT(status);

			if (elementIndex == influenceId) 
			{

				// Get weight and break loop
				//
				weights[i] = element.asDouble();
				break;

			}

		}

	}

	return MS::kSuccess;

};


MStatus TransferPaintWeightsCmd::applyPaintWeights(const MDagPath &mesh, const MDoubleArray &weights, const MColorArray &gradient, MString colorSetName)
/**
Modifies the vertex colour indices to reflect the supplied weights.
If the specified set cannot be found then a new one is created in its place.

@param mesh: The mesh to modify.
@param weights: The skin weights to derive the colour indices from.
@param gradient: The range of colours we can utilize.
@param colorSetName: The colour set to modify.
@return: Status code..
*/
{

	MStatus status;

	// Initialize function set
	//
	MFnDagNode fnDagNode(mesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Block the node from updating
	// This is really important in order to propogate changes!
	//
	MPlug nodeStatePlug = fnDagNode.findPlug("nodeState", &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	status = nodeStatePlug.setInt(2); // Blocking
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Initialize function set
	//
	MFnMesh fnMesh(mesh, &status);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Check if color set exists
	//
	bool hasColorSet = TransferPaintWeightsCmd::hasColorSet(mesh, colorSetName);

	if (!hasColorSet) 
	{

		colorSetName = fnMesh.createColorSetWithName(colorSetName, NULL, 0, &status);
		CHECK_MSTATUS_AND_RETURN_IT(status);

	}

	status = fnMesh.setCurrentColorSetName(colorSetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Clear any existing colors
	//
	status = fnMesh.clearColors(&colorSetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Set color ids using gradient
	//
	status = fnMesh.setColors(gradient, &colorSetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Assign colors ids based on weights
	//
	MIntArray colorIds = TransferPaintWeightsCmd::createColorIds(mesh, weights, gradient);

	status = fnMesh.assignColors(colorIds, &colorSetName);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	// Reset node state
	//
	status = nodeStatePlug.setInt(0); // Normal
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;

};


int TransferPaintWeightsCmd::clampValue(int value, int min, int max) 
/**
Clamps the specified value between the minimum and maximum values.

@param value: The value to clamp.
@param min: The minimum value.
@param max: The maximum value.
@return: The clamped value.
*/
{


	if (value < min) 
	{

		return min;

	}
	else if (value > max) 
	{

		return max;

	}
	else;

	return value;

};


MIntArray TransferPaintWeightsCmd::range(unsigned int start, unsigned int end, unsigned int step)
/**
Returns a range of integers based on the specified variables.

@param start: The starting integer.
@param end: The ending integer.
@param step: The increment for each integer.
@return: An array of integers.
*/
{

	unsigned int length = (end - start) / step;
	MIntArray range(length, 0);

	for (unsigned int i = 0; i < length; i += step) 
	{

		range[i] = i + start;

	}

	return range;

};


MStatus TransferPaintWeightsCmd::createGradient(const MString colorRamp, MColorArray &gradient)
/**
Populates the passed gradient using the supplied color ramp string.
Color ramp strings must follow a specific syntax that consists of numbers and comma delimiters.
These numbers must be divisible by 5 or else this method will fail!
Each color chunk consists of the following: Red, Green, Blue, Alpha, Position.

@param colorRamp: The color ramp string to parse.
@param gradient: The passed gradient object to populate.
@return: Status code..
*/
{

	MStatus status;

	// Consume all characters
	//
	const char* chars = colorRamp.asChar();

	std::string str(chars);
	std::stringstream ss(str);

	std::vector<float> vec;
	float i;

	while (ss >> i) 
	{

		vec.push_back(i);

		if (ss.peek() == DELIMITER) 
		{
			
			ss.ignore();
			
		}

	}

	ss.clear();

	// Check if color ramp is divisible by chunk size
	//
	size_t size = vec.size();

	int numerator = static_cast<int>(size);
	int denominator = static_cast<int>(CHUNK_SIZE);

	std::div_t result = std::div(numerator, denominator);

	if (result.rem != 0) {

		status = MS::kFailure;
		status.perror("Unable to evaluate supplied color ramp string!");

		return status;

	}

	// Initialize color slots
	//
	int numChunks = result.quot;
	std::vector<ColorSlot> slots(numChunks);

	unsigned int insertAt = 0;

	for (unsigned int i = 0; i < size; i += CHUNK_SIZE) 
	{

		slots[insertAt] = ColorSlot(vec[i], vec[i+1], vec[i+2], vec[i+3]);
		insertAt += 1;

	}

	std::sort(slots.begin(), slots.end(), less_than_key());

	// Lerp color slots
	// Must use inclusive range!
	//
	status = gradient.setLength(101);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	int start, end;
	float percent;

	for (int i = 0; i < (numChunks - 1); i++) 
	{

		// Get lerp range
		//
		start = static_cast<int>(slots[i].position * 100.0);
		end = static_cast<int>(slots[i+1].position * 100.0);

		for (int j = start; j <= end; j++) 
		{
			
			// Interpolate colors
			//
			percent = static_cast<float>(j) / 100.0f;

			gradient[j].r = slots[i].color.r + (slots[i+1].color.r - slots[i].color.r) * percent;
			gradient[j].g = slots[i].color.g + (slots[i+1].color.g - slots[i].color.g) * percent;
			gradient[j].b = slots[i].color.b + (slots[i+1].color.b - slots[i].color.b) * percent;
			gradient[j].a = 1.0f;

		}

	}

	return MS::kSuccess;

};


MSyntax TransferPaintWeightsCmd::newSyntax()
/**
Returns a syntax object that can parse this command.

@return: The syntax.
*/
{

	MStatus status;

	// Define optional flags
	//
	MSyntax syntax;

	status = syntax.addFlag("-csn", "-colorSetName", MSyntax::kString);
	CHECK_MSTATUS(status);

	status = syntax.addFlag("-cr", "-colorRamp", MSyntax::kString);
	CHECK_MSTATUS(status);

	status = syntax.addFlag("-rmc", "-rampMinColor", MSyntax::kDouble);
	CHECK_MSTATUS(status);

	status = syntax.addFlag("-rxc", "-rampMaxColor", MSyntax::kDouble);
	CHECK_MSTATUS(status);

	// Set multi-use flags
	//
	status = syntax.makeFlagMultiUse("-rmc");
	CHECK_MSTATUS(status);

	status = syntax.makeFlagMultiUse("-rxc");
	CHECK_MSTATUS(status);

	// Define arguments
	//
	syntax.useSelectionAsDefault(false);

	status = syntax.setObjectType(MSyntax::kSelectionList, 2, 2);
	CHECK_MSTATUS(status);

	// Define extended behavior
	//
	syntax.enableEdit(false);
	syntax.enableQuery(false);

	return syntax;

};


bool TransferPaintWeightsCmd::hasSyntax()
/**
Evaluates if this command uses syntax.

@return: Boolean.
*/
{

	return true;

};


void* TransferPaintWeightsCmd::creator() 
/**
Returns a new instance of this command.

@return: A new instance.
*/
{

	return new TransferPaintWeightsCmd();

};