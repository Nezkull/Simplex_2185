#include "MyRigidBody.h"
using namespace Simplex;
//Accessors
bool MyRigidBody::GetVisible(void) { return m_bVisible; }
float MyRigidBody::GetRadius(void) { return m_fRadius; }
vector3 MyRigidBody::GetColor(void) { return m_v3Color; }
void MyRigidBody::SetColor(vector3 a_v3Color) { m_v3Color = a_v3Color; }
vector3 MyRigidBody::GetCenterLocal(void) { return m_v3Center; }
vector3 MyRigidBody::GetMinLocal(void) { return m_v3MinL; }
vector3 MyRigidBody::GetMaxLocal(void) { return m_v3MaxL; }
vector3 MyRigidBody::GetCenterGlobal(void){	return vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)); }
vector3 MyRigidBody::GetMinGlobal(void) { return m_v3MinG; }
vector3 MyRigidBody::GetMaxGlobal(void) { return m_v3MaxG; }
vector3 MyRigidBody::GetHalfWidth(void) { return m_v3HalfWidth; }
matrix4 MyRigidBody::GetModelMatrix(void) { return m_m4ToWorld; }
void MyRigidBody::SetModelMatrix(matrix4 a_m4ModelMatrix) 
{
	

	m_v3CenterG = static_cast<vector3>(m_m4ToWorld * vector4(m_v3Center, 1.0f));

	m_v3MaxG = static_cast<vector3>(m_m4ToWorld * vector4(m_v3MaxL, 1.0f));
	m_v3MinG = static_cast<vector3>(m_m4ToWorld * vector4(m_v3MinL, 1.0f));


	if (a_m4ModelMatrix == m_m4ToWorld) {
		return;
	}

	m_m4ToWorld = a_m4ModelMatrix;

	vector3 v3Vertices[8];

	v3Vertices[0] = m_v3MinL;
	v3Vertices[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	v3Vertices[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	v3Vertices[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);

	v3Vertices[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Vertices[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Vertices[6] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Vertices[7] = m_v3MaxL;

	for (uint i = 0; i < 8; i++) {
		v3Vertices[i] = vector3(m_m4ToWorld * vector4(v3Vertices[i], 1.0f));
	}

	m_v3MaxG = m_v3MinG = v3Vertices[0];

	for (uint i = 0; i < 8; i++) {
		
		if (m_v3MaxG.x < v3Vertices[i].x) {
			m_v3MaxG.x = v3Vertices[i].x;
		}
		else if (m_v3MinG.x > v3Vertices[i].x) {
			m_v3MinG.x = v3Vertices[i].x;
		}

		if (m_v3MaxG.y < v3Vertices[i].y) {
			m_v3MaxG.y = v3Vertices[i].y;
		}
		else if (m_v3MinG.y > v3Vertices[i].y) {
			m_v3MinG.y = v3Vertices[i].y;
		}

		if (m_v3MaxG.z < v3Vertices[i].z) {
			m_v3MaxG.z = v3Vertices[i].z;
		}
		else if (m_v3MinG.z > v3Vertices[i].z) {
			m_v3MinG.z = v3Vertices[i].z;
		}
	}

	m_v3ARBBSize = m_v3MaxG - m_v3MinG;
}
//Allocation
void MyRigidBody::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_bVisible = true;

	m_fRadius = 0.0f;

	m_v3Color = C_WHITE;

	m_v3Center = ZERO_V3;
	m_v3MinL = ZERO_V3;
	m_v3MaxL = ZERO_V3;

	m_v3MinG = ZERO_V3;
	m_v3MaxG = ZERO_V3;

	m_v3HalfWidth = ZERO_V3;

	m_m4ToWorld = IDENTITY_M4;
}
void MyRigidBody::Swap(MyRigidBody& other)
{
	std::swap(m_pMeshMngr , other.m_pMeshMngr);
	std::swap(m_bVisible , other.m_bVisible);

	std::swap(m_fRadius, other.m_fRadius);

	std::swap(m_v3Color , other.m_v3Color);

	std::swap(m_v3Center , other.m_v3Center);
	std::swap(m_v3MinL , other.m_v3MinL);
	std::swap(m_v3MaxL , other.m_v3MaxL);

	std::swap(m_v3MinG , other.m_v3MinG);
	std::swap(m_v3MaxG , other.m_v3MaxG);

	std::swap(m_v3HalfWidth , other.m_v3HalfWidth);

	std::swap(m_m4ToWorld , other.m_m4ToWorld);
}
void MyRigidBody::Release(void)
{
	m_pMeshMngr = nullptr;
}
//The big 3
MyRigidBody::MyRigidBody(std::vector<vector3> a_pointList)
{
	Init();
	
	uint uPointCount = a_pointList.size();
	
	if (uPointCount < 3)
		return;

	m_v3MinL = m_v3MaxL = a_pointList[0];

	for (uint i = 1; i < uPointCount; i++) {
		if (m_v3MinL.x > a_pointList[i].x) {
			m_v3MinL.x = a_pointList[i].x;
		}
		else if (m_v3MaxL.x < a_pointList[i].x) {
			m_v3MaxL.x = a_pointList[i].x;
		}

		if (m_v3MinL.y > a_pointList[i].y) {
			m_v3MinL.y = a_pointList[i].y;
		}
		else if (m_v3MaxL.y < a_pointList[i].y) {
			m_v3MaxL.y = a_pointList[i].y;
		}

		if (m_v3MinL.z > a_pointList[i].z) {
			m_v3MinL.z = a_pointList[i].z;
		}
		else if (m_v3MaxL.z < a_pointList[i].z) {
			m_v3MaxL.z = a_pointList[i].z;
		}
	}
	
	m_v3MinG = m_v3MinL;
	m_v3MaxG = m_v3MaxL;
	
	for (int i = 0; i < uPointCount; i++) {
		m_v3Center += a_pointList[i];
	}

	m_v3Center = (m_v3MaxL + m_v3MinL) / 2.0f;
	m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	for (int i = 0; i < uPointCount; i++) {
		
		float fDistance = glm::distance(m_v3Center, a_pointList[i]);

		if (fDistance > m_fRadius) {
			m_fRadius = fDistance;
		}
	}

	// m_v3HalfWidth.x = glm::distance(vector3(m_v3MinL.x, 0.0f, 0.0f), vector3(m_v3MaxL.x, 0.0f, 0.0f));
	// m_v3HalfWidth.y = glm::distance(vector3(0.0f, m_v3MinL.y, 0.0f), vector3(0.0f, m_v3MaxL.y, 0.0f));
	// m_v3HalfWidth.z = glm::distance(vector3(0.0f, 0.0f, m_v3MinL.z), vector3(0.0f, 0.0f, m_v3MaxL.z));

	// m_fRadius = 10.0f;
	// m_v3Center = vector3(0.0f);
	// m_v3MinG = m_v3MinL = vector3(5.0f);
	// m_v3MaxG = m_v3MaxL = vector3(-5.0f);

	// m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	// m_fRadius = glm::distance(m_v3Center, m_v3MaxG);

	
}
MyRigidBody::MyRigidBody(MyRigidBody const& other)
{
	m_pMeshMngr = m_pMeshMngr;
	m_bVisible = m_bVisible;

	m_fRadius = m_fRadius;

	m_v3Color = m_v3Color;

	m_v3Center = m_v3Center;
	m_v3MinL = m_v3MinL;
	m_v3MaxL = m_v3MaxL;

	m_v3MinG = m_v3MinG;
	m_v3MaxG = m_v3MaxG;

	m_v3HalfWidth = m_v3HalfWidth;

	m_m4ToWorld = m_m4ToWorld;
}
MyRigidBody& MyRigidBody::operator=(MyRigidBody const& other)
{
	if(this != &other)
	{
		Release();
		Init();
		MyRigidBody temp(other);
		Swap(temp);
	}
	return *this;
}
MyRigidBody::~MyRigidBody(){Release();};

//--- Non Standard Singleton Methods
void MyRigidBody::AddToRenderList(void)
{
	
	// m_pMeshMngr->AddWireCubeToRenderList(glm::scale(m_v3HalfWidth), C_RED, RENDER_WIRE);
	if (!m_bVisible)
		return;

	// m_pMeshMngr->AddWireSphereToRenderList(glm::scale(vector3(m_fRadius)), C_RED, RENDER_WIRE);
	// m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), m_v3Color, RENDER_WIRE);
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3Color, RENDER_WIRE);
}
bool MyRigidBody::IsColliding(MyRigidBody* const other)
{
	// bool bIsColliding = true;

	// multiply by modelmatrix to go from local to global space
	//MxMy---MxMy
	// |	  |
	//MxMy---MxMy

	// bounding box 
	if (m_v3MaxG.x < other->m_v3MinG.x)
		return false;
	if (m_v3MinG.x > other->m_v3MaxG.x)
		return false;

	if (m_v3MaxG.y < other->m_v3MinG.y)
		return false;
	if (m_v3MinG.y > other->m_v3MaxG.y)
		return false;

	if (m_v3MaxG.z < other->m_v3MinG.z)
		return false;
	if (m_v3MinG.z > other->m_v3MaxG.z)
		return false;


	// bounding spheres
	//if (glm::distance(m_v3CenterG, other->m_v3CenterG) < m_fRadius + m_fRadius)
		//return true;



	return true;
}