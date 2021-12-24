#pragma once
#include <glm/glm.hpp>
#include <string>
#include "Face.h"
#include <math.h>

class MeshModel
{
public:
	MeshModel(std::vector<Face> faces, std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, const std::string& model_name);
	virtual ~MeshModel();
	const Face& GetFace(int index) const;
	int GetFacesCount() const;
	const std::string& GetModelName() const;

	void GetTransform();
	std::vector<glm::vec3> Draw(glm::mat4x4 transformMat);

	/*
	use these variables to do transformations
	less code clutter and easier implementation
	final matrices are calculaten in the GetTransform function
	*/

	float localTranslateArray[3] = { 0, 0, 0 };
	float localRotateArray[3] = { 0, 0, 0 };
	float localScaleArray[3] = { 1, 1, 1 };
	bool lockLocalScale = true;
	float localScaleLocked = 0.01;

	float worldTranslateArray[3] = { 0, 0, 0 };
	float worldRotateArray[3] = { 0, 0, 0 };
	float worldScaleArray[3] = { 1, 1, 1 };
	bool lockWorldScale = true;
	float worldScaleLocked = 1;

	bool drawAxis = false;
	bool drawBoundingBox = false;
	bool drawVertexNormals = false;
	bool drawFaceNormals = false;

	float vertexNormalsScale = 0.05;
	float faceNormalsScale = 0.001;

	int LocalWorldEditObject = 1;

	bool trianglesBoundingBoxes = false;
	bool coloredTriangles = false;
	bool trianglesOutlines = true;


	int maxScale = 0;

	float maxZ = -1.0f * FLT_MAX;
	float minZ = FLT_MAX;

	std::vector<glm::vec3> boundingBox;
	std::vector<glm::vec3> axis;

	std::vector<glm::vec3> transformedBoundingBox;
	std::vector<glm::vec3> transformedAxis;
	std::vector<std::vector<glm::vec3>> transformedVertexNormals;
	std::vector<std::vector<glm::vec3>> transformedFaceNormals;

	std::vector<Face> faces;
	std::vector<glm::vec3> vertices;

	glm::mat4x4 objectTransform;
	glm::mat4x4 objectTransformation = glm::mat4x4(1.0f);

	glm::vec3 color = glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
private:

	float maxCoordinates[3] = { -1.0f * FLT_MAX, -1.0f * FLT_MAX, -1.0f * FLT_MAX };
	float minCoordinates[3] = { FLT_MAX, FLT_MAX, FLT_MAX };

	// local matrices (init with identity matrix)
	glm::mat4x4 localTranslateMat = glm::mat4x4(1.0f);
	glm::mat4x4 localRotateXMat = glm::mat4x4(1.0f);
	glm::mat4x4 localRotateYMat = glm::mat4x4(1.0f);
	glm::mat4x4 localRotateZMat = glm::mat4x4(1.0f);
	glm::mat4x4 localScaleMat = glm::mat4x4(1.0f);

	// world matrices (init with identity matrix)
	glm::mat4x4 worldTranslateMat = glm::mat4x4(1.0f);
	glm::mat4x4 worldRotateXMat = glm::mat4x4(1.0f);
	glm::mat4x4 worldRotateYMat = glm::mat4x4(1.0f);
	glm::mat4x4 worldRotateZMat = glm::mat4x4(1.0f);
	glm::mat4x4 worldScaleMat = glm::mat4x4(1.0f);

	// Transformation matrices
	glm::mat4x4 axisTransformation = glm::mat4x4(1.0f);

	
	std::vector<glm::vec3> normals;
	std::string model_name;
	glm::mat4x4 worldTransform;
};
