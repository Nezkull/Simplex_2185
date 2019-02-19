#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader);

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));

	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
/*
	// Generate Circle code
	vector3 centerPoint = glm::vec3(0.0f, 0.0f, 0.0f); // set center point of the circle
	vector3 botLeftPt = glm::vec3(0.0f, 0.0f, 0.0f);
	vector3 botRightPt = glm::vec3(0.0f, 0.0f, 0.0f);

	float triSection = 2 * PI / a_nSubdivisions; // calculate theta

	for (int i = 0; i < a_nSubdivisions + 1; i++){
		float angle = triSection * i; // calculate angle
		float xPoint = centerPoint.x + a_fRadius * cosf(angle); // generate xPoint for triangle
		float yPoint = centerPoint.y + a_fRadius * sinf(angle); // generate yPoint for triangle

		botRightPt = glm::vec3(xPoint, yPoint, 0.0f); // creating bottom right triangle point

		AddTri(botLeftPt, botRightPt, centerPoint); // left right center

		// botLeftPt = glm::vec3(botRightPt.x, botRightPt.y, botRightPt.z); // setting bottom left triangle point to bottom right triangle point
		botLeftPt = botRightPt;
	}
*/
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	// GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	vector3 centerPoint = glm::vec3(0.0f); // set center point of the circle
	vector3 topCenterPoint = glm::vec3(0.0f, 0.0f, a_fHeight); // set center of upper circle
	vector3 botLeftPt = glm::vec3(0.0f);
	vector3 botRightPt = glm::vec3(0.0f);

	float triSection = 2 * PI / a_nSubdivisions; // calculate theta
	float angle = 0.0f;
	float xPoint = 0.0f;
	float yPoint = 0.0f;

	// probably need to draw the inverse of this
	// draws base
	for (int i = 0; i <= a_nSubdivisions; i++) {
		angle = triSection * i; // calculate angle
		xPoint = centerPoint.x + a_fRadius * cosf(angle); // generate xPoint for triangle
		yPoint = centerPoint.y + a_fRadius * sinf(angle); // generate yPoint for triangle

		botRightPt = glm::vec3(xPoint, yPoint, 0.0f); // creating bottom right triangle point

		if (i != 0) {
			AddTri(botRightPt, botLeftPt, centerPoint); // right left center
			AddTri(botLeftPt, botRightPt, topCenterPoint); // left right center
		}

		botLeftPt = botRightPt;
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	// GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// -------------------------------

	vector3 centerPoint = glm::vec3(0.0f); // set center point of the circle
	vector3 topCenterPoint = glm::vec3(0.0f, 0.0f, a_fHeight); // set center of upper circle
	vector3 botLeftPt = glm::vec3(0.0f);
	vector3 botRightPt = glm::vec3(0.0f);
	vector3 topLeftPt = glm::vec3(0.0f);
	vector3 topRightPt = glm::vec3(0.0f);

	float triSection = 2 * PI / a_nSubdivisions; // calculate theta
	float angle = 0.0f;
	float xPoint = 0.0f;
	float yPoint = 0.0f;

	for (int i = 0; i <= a_nSubdivisions; i++) {
		angle = triSection * i; // calculate angle
		xPoint = centerPoint.x + a_fRadius * cosf(angle); // generate xPoint for triangle
		yPoint = centerPoint.y + a_fRadius * sinf(angle); // generate yPoint for triangle

		botRightPt = glm::vec3(xPoint, yPoint, 0.0f); // creating bottom right triangle point
		topRightPt = glm::vec3(xPoint, yPoint, a_fHeight);

		if (i != 0) {
			AddTri(botRightPt, botLeftPt, centerPoint); // left right center
			AddTri(topLeftPt, topRightPt, topCenterPoint); // left right, center
			AddQuad(botLeftPt, botRightPt, topLeftPt, topRightPt); // botLeft, botRight, topLeft, topRight
		}

		botLeftPt = botRightPt;
		topLeftPt = topRightPt;
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	// GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	vector3 centerPoint = glm::vec3(0.0f); // set center point of the circle
	vector3 topCenterPoint = glm::vec3(0.0f, 0.0f, a_fHeight); // set center of upper circle
	vector3 botLeftPt = glm::vec3(0.0f);
	vector3 botRightPt = glm::vec3(0.0f);
	vector3 topLeftPt = glm::vec3(0.0f);
	vector3 topRightPt = glm::vec3(0.0f);
	vector3 botInnerLeft = glm::vec3(0.0f);
	vector3 botInnerRight = glm::vec3(0.0f);
	vector3 topInnerLeft = glm::vec3(0.0f);
	vector3 topInnerRight = glm::vec3(0.0f);

	float triSection = 2 * PI / a_nSubdivisions; // calculate theta
	float angle = 0.0f;
	float xOuterPoint = 0.0f;
	float yOuterPoint = 0.0f;
	float xInnerPoint = 0.0f;
	float yInnerPoint = 0.0f;

	for (int i = 0; i <= a_nSubdivisions; i++) {
		angle = triSection * i; // calculate angle
		xOuterPoint = centerPoint.x + a_fOuterRadius * cosf(angle); // generate xPoint for triangle
		yOuterPoint = centerPoint.y + a_fOuterRadius * sinf(angle); // generate yPoint for triangle

		botRightPt = glm::vec3(xOuterPoint, yOuterPoint, 0.0f); // creating bottom right triangle point
		topRightPt = glm::vec3(xOuterPoint, yOuterPoint, a_fHeight);

		if (i != 0) {
			AddQuad(botLeftPt, botRightPt, topLeftPt, topRightPt); // this works draws outer quads
		}

		xInnerPoint = centerPoint.x + a_fInnerRadius * cosf(angle); // generate xPoint for triangle
		yInnerPoint = centerPoint.y + a_fInnerRadius * sinf(angle); // generate yPoint for triangle

		botInnerRight = glm::vec3(xInnerPoint, yInnerPoint, 0.0f); // creating bottom right triangle point
		topInnerRight = glm::vec3(xInnerPoint, yInnerPoint, a_fHeight);

		if (i != 0) {
			AddQuad(botInnerRight, botInnerLeft, topInnerRight, topInnerLeft); // this works draws inner quads
			AddQuad(topLeftPt, topRightPt, topInnerLeft, topInnerRight); // top draw
			AddQuad(botRightPt, botLeftPt, botInnerRight, botInnerLeft); // bottom draw
		}

		// point transfer code stuff
		botLeftPt = botRightPt;
		topLeftPt = topRightPt;
		botInnerLeft = botInnerRight;
		topInnerLeft = topInnerRight;
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// -------------------------------

	// inner is the radius of the tube
	// b is the subdivisions of the tube

	// save start points into a vector
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}

//http://www.songho.ca/opengl/gl_sphere.html followed along to this for help 
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// variable declarations
	float x; // x value for coordinate
	float y; // y value for coordinate
	float z; // z value for coordinate
	float xy; // xy value for coordinate that changes in the second for loop
	float countVal = a_nSubdivisions * 3; // increases the amount of subdivisions to make it look smoother/nicer
	float sectorStep = 2 * PI / countVal; // latitude
	float stackStep = PI / countVal; // longitude
	float sectorAngle = 0.0f; // angle for the latitude
	float stackAngle = 0.0f; // angle for longitude
	int k1 = 0; // placeholder value for point variable
	int k2 = 0; // placeholder value for point variable
	std::vector<vector3> points; // vector of points in the sphere
	std::vector<int> indices; // vector of int values that pertain to adjoining points in a row on the sphere

	// looping through increased subdivisions
	for (int i = 0; i <= countVal; i++) 
	{
		stackAngle = PI / 2 - i * stackStep; // calculating angle for longitude
		xy = a_fRadius * cosf(stackAngle); // calculates the base xy value that then gets split into seperate x & y values in the next loop
		z = a_fRadius * sinf(stackAngle); // calculates the z value for the points

		// looping through increased subdivisions
		for (int j = 0; j <= countVal; j++, k1++, k2++) 
		{
			sectorAngle = j * sectorStep; // calculating angle for latitude
			x = xy * cosf(sectorAngle); // calculating x value for point
			y = xy * sinf(sectorAngle); // calculating y value for point
			points.push_back(vector3(x, y, z)); // adding a tri to the points vector
		}
	}

	// loops through increased subdivisions
	for (int i = 0; i < countVal; i++) 
	{
		k1 = i * (countVal + 1); // setting initial value for points variable
		k2 = k1 + countVal + 1; // setting initial value for points variable

		// loops through increased subdivisions
		for (int j = 0; j < countVal; j++) 
		{
			// top and bottom of sphere
			if (i != 0) 
			{
				AddTri(points[k1], points[k2], points[k1 + 1]); // adding a triangle to a the render list
			}

			// innards of the sphere
			if (i != (countVal - 1)) 
			{
				AddTri(points[k1 + 1], points[k2], points[k2 + 1]); // adding a triangle to a the render list
			}

			k1++; // incrementing a points variable
			k2++; // incrementing othe points variable
		}
	}
	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}