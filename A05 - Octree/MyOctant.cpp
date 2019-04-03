#include "MyOctant.h"

using namespace Simplex;

// My Octant
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;
uint MyOctant::GetOctantCount() { 
	return m_uOctantCount; 
}

void MyOctant::Init() {
	m_uChildren = 0;

	m_fSize = 0;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0);
	m_v3Min = vector3(0);
	m_v3Max = vector3(0);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (int i = 0; i < 8; i++) {
		m_pChild[i] = nullptr;
	}
}

void MyOctant::Swap(MyOctant & other) {
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++) {
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

MyOctant * MyOctant::GetParent() {
	return m_pParent;
}

void MyOctant::Release() {
	// this is a special kind of release, it will only happen for the root
	if (m_uLevel == 0) {
		KillBranches();
	}

	m_uChildren = 0;
	m_fSize = 0;
	m_EntityList.clear();
	m_lChild.clear();
}

// The Big 3
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount) {
	// Init the default values
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax; // this will hold all of the Min and Max vectors of the Bounding Object

	uint nObjects = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < nObjects; i++) {
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal());
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}
	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 v3HalfWidth = pRigidBody->GetHalfWidth();
	float fMax = v3HalfWidth.x;
	for (uint i = 0; i < 3; i++) {
		if (fMax < v3HalfWidth[i]) {
			fMax = v3HalfWidth[i];
		}
	}
	vector3 v3Center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = fMax * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize) {
	// Init the default values
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2);

	m_uOctantCount++;
}

MyOctant::MyOctant(MyOctant const & other) {
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++) {
		m_pChild[i] = other.m_pChild[i];
	}
}

MyOctant & MyOctant::operator=(MyOctant const & other) {
	if (this != &other) {
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}

	return *this;
}

MyOctant::~MyOctant() {
	Release();
}

// Accessors
float MyOctant::GetSize() {
	return m_fSize;
}

vector3 MyOctant::GetCenterGlobal() {
	return m_v3Center;
}

vector3 MyOctant::GetMinGlobal() {
	return m_v3Min;
}

vector3 MyOctant::GetMaxGlobal() {
	return m_v3Max;
}

// --- Non Standard Singleton Methods
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color) {
	if (m_uID == a_nIndex) {
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);

		return;
	}

	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color) {
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::Subdivide() {
	// If this node has reached the maximum depth, return without changes
	if (m_uLevel >= m_uMaxLevel) {
		return;
	}

	// If this node has already been subdivided, return without changes
	if (m_uChildren != 0) {
		return;
	}

	// As this will have children, it will not be a leaf
	m_uChildren = 8;

	float fSize = m_fSize / 4;
	float fSizeD = fSize * 2;
	vector3 v3Center;

	// BOTTOM OCTANTS
	// MyOctant 0 bottom Left Back
	v3Center = m_v3Center;
	v3Center.x -= fSize;
	v3Center.y -= fSize;
	v3Center.z -= fSize;
	m_pChild[0] = new MyOctant(v3Center, fSizeD);

	// MyOctant 1 Bottom Right Back
	v3Center.x += fSizeD;
	m_pChild[1] = new MyOctant(v3Center, fSizeD);

	// MyOctant 2 Bottom Right Front
	v3Center.z += fSizeD;
	m_pChild[2] = new MyOctant(v3Center, fSizeD);

	// MyOctant 3 Bottom Left Front
	v3Center.x -= fSizeD;
	m_pChild[3] = new MyOctant(v3Center, fSizeD);

	// TOP OCTANTS
	// MyOctant 4 Top Left Front
	v3Center.y += fSizeD;
	m_pChild[4] = new MyOctant(v3Center, fSizeD);

	// MyOctant 5 Top Left Back
	v3Center.z -= fSizeD;
	m_pChild[5] = new MyOctant(v3Center, fSizeD);
	
	// MyOctant 6 Top Right Back
	v3Center.x += fSizeD;
	m_pChild[6] = new MyOctant(v3Center, fSizeD);

	// MyOctant 7 Top Right Front
	v3Center.z += fSizeD;
	m_pChild[7] = new MyOctant(v3Center, fSizeD);

	for (uint i = 0; i < 8; i++) {
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;

		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount)) {
			m_pChild[i]->Subdivide();
		}
	}
}

MyOctant * MyOctant::GetChild(uint a_nChild) {
	if (a_nChild > 7) {
		return nullptr;
	}
	return m_pChild[a_nChild];
}

bool MyOctant::IsColliding(uint a_uRBIndex) {
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	
	// If the given index is larger than the number of elements in the Bounding Object, there is no collision
	if (a_uRBIndex >= nObjectCount) {
		return false;
	}

	// As the Octree will never rotate or scale, this collision is as easy as an AABB
	// Get all vectors in global space (the octant ones are already in Global)
	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinO = pRigidBody->GetMinGlobal();
	vector3 v3MaxO = pRigidBody->GetMaxGlobal();

	// Check for x
	if (m_v3Max.x < v3MinO.x) {
		return false;
	}
	if (m_v3Min.x > v3MaxO.x) {
		return false;
	}

	// Check for y
	if (m_v3Max.y < v3MinO.y) {
		return false;
	}
	if (m_v3Min.y > v3MaxO.y) {
		return false;
	}

	// Check for z
	if (m_v3Max.z < v3MinO.z) {
		return false;
	}
	if (m_v3Min.z > v3MaxO.z) {
		return false;
	}

	return true;
}

bool MyOctant::IsLeaf() {
	return m_uChildren == 0;
}

bool MyOctant::ContainsMoreThan(uint a_nEntities) {
	uint nCount = 0;
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	
	for (uint i = 0; i < nObjectCount; i++) {
		if (IsColliding(i)) {
			nCount++;
		}

		if (nCount > a_nEntities) {
			return true;
		}
	}
	
	return false;
}

void MyOctant::KillBranches() {
	/*  
		This method has recursive behavior that is somewhat hard to explain line by line but
		it will traverse the whole tree until it reaches a node with no children and once it 
		returns from it to its parent, it will kill all of its children recursively until it reaches the node that called it.
	*/

	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctant::DisplayLeafs(vector3 a_v3Color) {
	uint nLeafs = m_lChild.size();
	
	for (uint i = 0; i < nLeafs; i++) {
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::ClearEntityList() {
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ClearEntityList();
	}
}

void MyOctant::ConstructTree(uint a_nMaxLevel) {
	// If this function is attempted to be applied to something other than the root, don't.
	if (m_uLevel != 0) {
		return;
	}

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	// ClearEntityList();
	m_EntityList.clear();

	// Clear the tree
	KillBranches();
	m_lChild.clear();

	// If the base tree
	if (ContainsMoreThan(m_uIdealEntityCount)) {
		Subdivide();
	}

	// Add octant ID to Entities
	AssignIDtoEntity();

	// Construct the list of Objects
	ConstructList();
}

void MyOctant::AssignIDtoEntity() {
	// Traverse until a leaf is reached
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->AssignIDtoEntity();
	}

	if (m_uChildren == 0) {
		uint nEntities = m_pEntityMngr->GetEntityCount();

		for (uint i = 0; i < nEntities; i++) {
			if (IsColliding(i)) {
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

void MyOctant::ConstructList() {
	for (uint i = 0; i < m_uChildren; i++) {
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0) {
		m_pRoot->m_lChild.push_back(this);
	}
}