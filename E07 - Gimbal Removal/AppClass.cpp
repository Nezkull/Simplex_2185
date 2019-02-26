
#include "AppClass.h"
void Application::InitVariables(void)
{
	// added for demo
	m_pCameraMngr->SetPositionTargetAndUpward(
		vector3(0.0f, 0.0f, 8.0f),
		vector3(0.0f),
		vector3(0.0f, 1.0f, 0.0f)
	);


	//init the mesh
	m_pMesh = new MyMesh();
	//m_pMesh->GenerateCube(1.0f, C_WHITE);
	m_pMesh->GenerateCone(2.0f, 5.0f, 3, C_WHITE);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// added for demo
	// float fRatio = 1080.0f / 720.0f;
	float fRatio = m_pSystem->GetWindowWidth() / static_cast<float>(m_pSystem->GetWindowHeight()); 
	float fNear = 0.01f;
	float fFar = 1000.0f;

	// added for demo
	matrix4 m4Projection = glm::perspective(m_fFov, fRatio, fNear, fFar);
	m4Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, fNear, fFar);

	
	vector3 v3Center = m_v3Eye + vector3(0.0f,0.0f, -1.0f);
	vector3 v3Up = vector3(0.0f,1.0f,0.0f); // point just infront of my "eye" 30%of the homework


	// Clear the screen
	ClearScreen();


	matrix4 m4View = glm::lookAt(m_v3Eye, v3Center, v3Up); // changed for the demo/homework

	// matrix4 m4View = m_pCameraMngr->GetViewMatrix();
	// matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	/*
	m_m4Model = glm::rotate(IDENTITY_M4, glm::radians(m_v3Rotation.x), vector3(1.0f, 0.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.y), vector3(0.0f, 1.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.z), vector3(0.0f, 0.0f, 1.0f));
	m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_m4Model));
	*/
	glm::quat quat1 = glm::angleAxis(glm::radians(m_v3Rotation.x),AXIS_X);
	glm::quat quat2 = glm::angleAxis(glm::radians(m_v3Rotation.y), AXIS_Y);
	glm::quat quat3 = glm::angleAxis(glm::radians(m_v3Rotation.z), AXIS_Z);

	m_qOrientation = m_qOrientation * (quat1*quat2*quat3);
	// m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_qOrientation));
	m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_qArcBall)); // added for demo
	// m_pMesh->Render(m4Projection, m4View, glm::translate(vector3(0.0f, 0.0f, -50.0f)));
	
	m_v3Rotation = vector3(0.0f);

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}