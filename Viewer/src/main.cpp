#define _USE_MATH_DEFINES
#include <cmath>
#include <imgui/imgui.h>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nfd.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"
#include "Light.h"

/**
 * Fields
 */
bool show_demo_window = false;
bool show_another_window = false;
glm::vec4 clear_color = glm::vec4(0.117f, 0.117f, 0.117f, 0.117f);
ImVec2 oldCoordinates;
float oldMouseWheelPos;
int LocalWorldEditCamera = 1;
bool cameraT = true;
std::shared_ptr<MeshModel> cameraModel;

/**
 * Function declarations
 */
static void GlfwErrorCallback(int error, const char* description);
GLFWwindow* SetupGlfwWindow(int w, int h, const char* window_name);
ImGuiIO& SetupDearImgui(GLFWwindow* window);
void StartFrame();
void RenderFrame(GLFWwindow* window, Scene& scene, Renderer& renderer, ImGuiIO& io);
void Cleanup(GLFWwindow* window);
void DrawImguiMenus(ImGuiIO& io, Scene& scene);

/**
 * Function implementation
 */
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	// TODO: Handle mouse scroll here
}

int main(int argc, char** argv)
{
	int windowWidth = 1280, windowHeight = 1001;
	GLFWwindow* window = SetupGlfwWindow(windowWidth, windowHeight, "Mesh Viewer");
	if (!window)
		return 1;

	int frameBufferWidth, frameBufferHeight;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	Renderer renderer = Renderer(frameBufferWidth, frameBufferHeight);
	Scene scene = Scene();
	shared_ptr<Camera>& camera = std::make_shared<Camera>();
	scene.AddCamera(camera);
	cameraModel = Utils::LoadMeshModel("..\\Data\\camera.obj");

	ImGuiIO& io = SetupDearImgui(window);
	glfwSetScrollCallback(window, ScrollCallback);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		StartFrame();
		DrawImguiMenus(io, scene);
		RenderFrame(window, scene, renderer, io);
	}

	Cleanup(window);
	return 0;
}

static void GlfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* SetupGlfwWindow(int w, int h, const char* window_name)
{
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit())
		return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(w, h, window_name, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
						 // very importent!! initialization of glad
						 // https://stackoverflow.com/questions/48582444/imgui-with-the-glad-opengl-loader-throws-segmentation-fault-core-dumped

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	return window;
}

ImGuiIO& SetupDearImgui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();
	return io;
}

void StartFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void RenderFrame(GLFWwindow* window, Scene& scene, Renderer& renderer, ImGuiIO& io)
{
	ImGui::Render();
	int frameBufferWidth, frameBufferHeight;
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	if (frameBufferWidth != renderer.GetViewportWidth() || frameBufferHeight != renderer.GetViewportHeight())
	{
		// Set new aspect ratio
		//if (frameBufferWidth > 1080) {
		//	frameBufferWidth = 1080;
		//}
		//if (frameBufferHeight > 720) {
		//	frameBufferHeight = 720;
		//}
		//renderer.setViewportWidth(frameBufferWidth);
		//renderer.setViewportHeight(frameBufferHeight);

		Renderer renderer = Renderer(frameBufferWidth, frameBufferHeight);
	}

	if (!io.WantCaptureKeyboard)
	{

		// local translate
		// 'W': translate Y-axis up (local)
		if (io.KeysDown[87] || io.KeysDown[119])
		{
			if (scene.GetModelCount() > 0) {
				MeshModel& model = scene.GetActiveModel();
				model.localTranslateArray[1] += 20;
			}
		}

		// 'A': translate X-axis left (local)
		if (io.KeysDown[65] || io.KeysDown[97])
		{
			if (scene.GetModelCount() > 0) {
				MeshModel& model = scene.GetActiveModel();
				model.localTranslateArray[0] -= 20;
			}
		}

		// 'S': translate Y-axis down (local)
		if (io.KeysDown[83] || io.KeysDown[115])
		{
			if (scene.GetModelCount() > 0) {
				MeshModel& model = scene.GetActiveModel();
				model.localTranslateArray[1] -= 20;
			}
		}

		// 'D': translate X-axis right (local)
		if (io.KeysDown[68] || io.KeysDown[100])
		{
			if (scene.GetModelCount() > 0) {
				MeshModel& model = scene.GetActiveModel();
				model.localTranslateArray[0] += 20;
			}
		}

		//J
		if (io.KeysDown[74] || io.KeysDown[106])
		{
			if (scene.GetCameraCount() > 0) {
				Camera& camera = scene.GetActiveCamera();
				camera.IncrementalTrans(true);
			}
		}

		//L
		if (io.KeysDown[76] || io.KeysDown[108])
		{
			if (scene.GetCameraCount() > 0) {
				Camera& camera = scene.GetActiveCamera();
				camera.IncrementalTrans(false);
			}
		}
	}

	if (!io.WantCaptureMouse)
	{
		// TODO: Handle mouse events here
		if (io.MouseDown[0])
		{
			ImVec2 newCoordinates = io.MousePos;
			// Left mouse button is down
			if (scene.GetModelCount() > 0) {
				MeshModel& model = scene.GetActiveModel();
				model.localRotateArray[1] += (newCoordinates[0] - oldCoordinates[0]) / 10;
				model.localRotateArray[0] += (newCoordinates[1] - oldCoordinates[1]) / 10;

				// scene.GetActiveCamera().worldRotateArray[1] += (newCoordinates[0] - oldCoordinates[0]) / 10;
				// scene.GetActiveCamera().worldRotateArray[0] += (newCoordinates[1] - oldCoordinates[1]) / 10;
			}
			
			
		}
		oldCoordinates = io.MousePos;

		/*
		tried to use scroll wheel to scale the object.
		maybe implement it down the road.
		*/
		//if ((oldMouseWheelPos - io.MouseWheel) != 0) {
		//	if (scene.GetModelCount() > 0) {
		//		MeshModel& model = scene.GetActiveModel();
		//		model.localTranslateArray[0] += io.MouseWheel * 10;
		//		model.localTranslateArray[1] += io.MouseWheel * 10;
		//	}
		//}
		//oldMouseWheelPos = io.MouseWheel;


		//float scrollX = ImGui::GetScrollX();
		//float scrollY = ImGui::GetScrollY();
		//if (scrollY) {
		//	if (scene.GetModelCount() > 0) {
		//		MeshModel& model = scene.GetActiveModel();
		//		model.localScaleArray[0] += 2;
		//		model.localScaleArray[1] += 2;
		//		model.localScaleArray[2] += 2;
		//	}
		//}
	}

	renderer.ClearColorBuffer(clear_color);
	renderer.Render(scene, cameraModel);
	renderer.SwapBuffers();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwMakeContextCurrent(window);
	glfwSwapBuffers(window);
}

void Cleanup(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void DrawImguiMenus(ImGuiIO& io, Scene& scene)
{
	/**
	 * MeshViewer menu
	 */
	
	 // Menu Bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				nfdchar_t* outPath = NULL;
				nfdresult_t result = NFD_OpenDialog("obj;", NULL, &outPath);
				if (result == NFD_OKAY)
				{
					scene.AddModel(Utils::LoadMeshModel(outPath));
					free(outPath);
				}
				else if (result == NFD_CANCEL)
				{
				}
				else
				{
				}

			}

			ImGui::EndMenu();
		}

		// TODO: Add more menubar items (if you want to)
		ImGui::EndMainMenuBar();
	}
	
	

	/**
	 * Imgui demo - you can remove it once you are familiar with imgui
	 */

	 // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);


	
	int modelCount = scene.GetModelCount();
	
	ImGui::Begin("Transformations");

	if (ImGui::BeginTabBar("Transformations"))
	{
		if (ImGui::BeginTabItem("General")) {
			ImGui::ColorEdit3("Background color", (float*)&clear_color);
			ImGui::Text("FPS %.1f", ImGui::GetIO().Framerate);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Camera")) {

			if (ImGui::Button("Add Camera")) {
				shared_ptr<Camera>& newCam = std::make_shared<Camera>();
				scene.AddCamera(newCam);
			}

			ImGui::Text("Select Camera:");
			int numCameras = scene.GetCameraCount();


			static int selectedCamra = 0;
			for (int n = 0; n < numCameras; n++) {
				std::string camName = "Camera ";
				camName.append(std::to_string(n));
				char bufc[64];
				sprintf(bufc, camName.c_str(), n);
				if (ImGui::Selectable(bufc, selectedCamra == n))
					selectedCamra = n;
			}

			scene.SetActiveCameraIndex(selectedCamra);

			Camera& camera = scene.GetCamera(selectedCamra);

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			ImGui::InputFloat("Camera Scale", &(*cameraModel).localScaleLocked, 0.01, 0.01, "%.2f");

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("--- CAMERA LOCATION (EYE) ---");

			ImGui::InputFloat("Location X", &camera.eye[0], 0.1, 5, "%.2f");
			ImGui::InputFloat("Location Y", &camera.eye[1], 0.1, 5, "%.2f");
			ImGui::InputFloat("Location Z", &camera.eye[2], 0.1, 5, "%.2f");

			ImGui::Separator();

			ImGui::Text("--- CAMERA TARGET (AT) ---");

			ImGui::InputFloat("Target X", &camera.at[0], 0.1, 1, "%.2f");
			ImGui::InputFloat("Target Y", &camera.at[1], 0.1, 1, "%.2f");
			ImGui::InputFloat("Target Z", &camera.at[2], 0.1, 1, "%.2f");

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("Transformation Type:");
			ImGui::SameLine();
			ImGui::RadioButton("Local", &LocalWorldEditCamera, 1);
			ImGui::SameLine();
			ImGui::RadioButton("world", &LocalWorldEditCamera, 0);

			if (LocalWorldEditCamera) {
				ImGui::Text("--- LOCAL TRANSFORMATIONS ---");

				ImGui::Text("Local Translate:");
				ImGui::InputFloat("Local Translate X", &camera.localTranslateArray[0], 0.1, 5, "%.2f");
				ImGui::InputFloat("Local Translate Y", &camera.localTranslateArray[1], 0.1, 5, "%.2f");
				ImGui::InputFloat("Local Translate Z", &camera.localTranslateArray[2], 0.1, 5, "%.2f");

				ImGui::Separator();

				ImGui::Text("Local Rotate:");
				ImGui::InputFloat("Rotate X", &camera.localRotateArray[0], 0.1, 1, "%.2f");
				ImGui::InputFloat("Rotate Y", &camera.localRotateArray[1], 0.1, 1, "%.2f");
				ImGui::InputFloat("Rotate Z", &camera.localRotateArray[2], 0.1, 1, "%.2f");

				ImGui::Separator();
				ImGui::Separator();
				ImGui::Separator();
				ImGui::Separator();
			}
			else {
				ImGui::Text("--- WORLD TRANSFORMATIONS ---");
				ImGui::Text("World Translate:");
				ImGui::InputFloat("World Translate X", &camera.worldTranslateArray[0], 0.1, 5, "%.2f");
				ImGui::InputFloat("World Translate Y", &camera.worldTranslateArray[1], 0.1, 5, "%.2f");
				ImGui::InputFloat("World Translate Z", &camera.worldTranslateArray[2], 0.1, 5, "%.2f");

				ImGui::Separator();

				ImGui::Text("World Rotate:");
				ImGui::InputFloat("Rotate X", &camera.worldRotateArray[0], 0.1, 1, "%.2f");
				ImGui::InputFloat("Rotate Y", &camera.worldRotateArray[1], 0.1, 1, "%.2f");
				ImGui::InputFloat("Rotate Z", &camera.worldRotateArray[2], 0.1, 1, "%.2f");

				ImGui::Separator();
				ImGui::Separator();
				ImGui::Separator();
				ImGui::Separator();
			}

			ImGui::Text("View Type:");
			ImGui::SameLine();
			ImGui::RadioButton("Orthographic", &(camera.OrthoPerspective), 1);
			ImGui::SameLine();
			ImGui::RadioButton("Perspective", &(camera.OrthoPerspective), 0);

			if (camera.OrthoPerspective) {
				ImGui::Text("Orthographic Settings:");
				ImGui::InputFloat("Camera near", &(camera.nearVal), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera far", &(camera.farVal), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera top", &(camera.top), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera bottom", &(camera.bottom), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera right", &(camera.right), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera left", &(camera.left), 0.01f, 1.0f, "%.3f");
			}
			else {
				ImGui::InputFloat("Camera near", &(camera.nearVal), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera far", &(camera.farVal), 0.01f, 1.0f, "%.3f");
				ImGui::InputFloat("Camera FOV", &(camera.fov), 1, 1, "%.0f");
			}

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();


			ImGui::Text("Addons:");
			ImGui::Checkbox("Draw World Axis", &(scene.drawWorldAxis));

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Object"))
		{
			// display transformations window if at least one model is loaded
			if (modelCount) {

				ImGui::Text("Select Model:");

				static int selected = 0;
				for (int n = 0; n < modelCount; n++) {
					char buf[64];
					sprintf(buf, scene.modelsNames[n].c_str(), n);
					if (ImGui::Selectable(buf, selected == n))
						selected = n;
				}

				scene.SetActiveModelIndex(selected);
				MeshModel& model = scene.GetActiveModel();

				ImGui::Text("Transformation Type:");
				ImGui::SameLine();
				ImGui::RadioButton("Local", &(model.LocalWorldEditObject), 1);
				ImGui::SameLine();
				ImGui::RadioButton("world", &(model.LocalWorldEditObject), 0);


				if (model.LocalWorldEditObject) {
					ImGui::Text("--- LOCAL TRANSFORMATIONS ---");
					ImGui::Text("Local Scale:");
					ImGui::InputFloat("Local Scale X", &model.localScaleArray[0], 0.1f, 1.0f, "%.3f");
					ImGui::InputFloat("Local Scale Y", &model.localScaleArray[1], 0.1f, 1.0f, "%.3f");
					ImGui::InputFloat("Local Scale Z", &model.localScaleArray[2], 0.1f, 1.0f, "%.3f");

					ImGui::Separator();

					ImGui::Checkbox("Lock All Local Axis", &(model.lockLocalScale));
					ImGui::InputFloat("Local Scale Locked", &(model.localScaleLocked), 0.1f, 1.0f, "%.3f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();

					ImGui::Text("Local Translate:");
					ImGui::InputFloat("Local Translate X", &model.localTranslateArray[0], 0.05, 1, "%.2f");
					ImGui::InputFloat("Local Translate Y", &model.localTranslateArray[1], 0.05, 1, "%.2f");
					ImGui::InputFloat("Local Translate Z", &model.localTranslateArray[2], 0.05, 1, "%.2f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();

					ImGui::Text("Local Rotate:");
					ImGui::InputFloat("Local Rotate X", &model.localRotateArray[0], 10, 10, "%.0f");
					ImGui::InputFloat("Local Rotate Y", &model.localRotateArray[1], 10, 10, "%.0f");
					ImGui::InputFloat("Local Rotate Z", &model.localRotateArray[2], 10, 10, "%.0f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
				}
				else {
					ImGui::Text("--- WORLD TRANSFORMATIONS ---");
					ImGui::Text("World Scale:");
					ImGui::InputFloat("World Scale X", &model.worldScaleArray[0], 0.1f, 1.0f, "%.3f");
					ImGui::InputFloat("World Scale Y", &model.worldScaleArray[1], 0.1f, 1.0f, "%.3f");
					ImGui::InputFloat("World Scale Z", &model.worldScaleArray[2], 0.1f, 1.0f, "%.3f");

					ImGui::Separator();

					ImGui::Checkbox("Lock All World Axis", &(model.lockWorldScale));
					ImGui::InputFloat("World Scale Locked", &(model.worldScaleLocked), 0.1f, 1.0f, "%.3f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();


					ImGui::Text("World Translate:");
					ImGui::InputFloat("World Translate X", &model.worldTranslateArray[0], 0.05, 1, "%.2f");
					ImGui::InputFloat("World Translate Y", &model.worldTranslateArray[1], 0.05, 1, "%.2f");
					ImGui::InputFloat("World Translate Z", &model.worldTranslateArray[2], 0.05, 1, "%.2f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();

					ImGui::Text("World Rotate:");
					ImGui::InputFloat("World Rotate X", &model.worldRotateArray[0], 1, 1, "%.0f");
					ImGui::InputFloat("World Rotate Y", &model.worldRotateArray[1], 1, 1, "%.0f");
					ImGui::InputFloat("World Rotate Z", &model.worldRotateArray[2], 1, 1, "%.0f");

					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();
					ImGui::Separator();

				}
				ImGui::Text("Object Addons:");
				ImGui::ColorEdit3("Object Color", (float*)&model.color);
				ImGui::Separator();
				ImGui::ColorEdit3("Object ambient", (float*)&model.ambientColor);

				ImGui::Separator();
				ImGui::ColorEdit3("Object diffuse", (float*)&model.diffuseColor);

				ImGui::Separator();
				ImGui::ColorEdit3("Object specular", (float*)&model.specularColor);

				ImGui::Separator();
				ImGui::Checkbox("Draw Axis", &(model.drawAxis));
				ImGui::Separator();
				ImGui::Checkbox("Draw Bounding Box", &(model.drawBoundingBox));
				ImGui::Separator();
				ImGui::Checkbox("Draw Vertex Normals", &(model.drawVertexNormals));
				ImGui::InputFloat("Vertex Normals Scale", &(model.vertexNormalsScale), 0.05, 1, "%.2f");
				ImGui::Separator();
				ImGui::Checkbox("Draw Face Normals", &(model.drawFaceNormals));
				ImGui::InputFloat("Face Normals Scale", &(model.faceNormalsScale), 0.0001, 1, "%.4f");
				ImGui::Separator();
				ImGui::Checkbox("Triangles Bounding Boxes", &(model.trianglesBoundingBoxes));
				ImGui::Separator();
				ImGui::Checkbox("Colored Triangles", &(model.coloredTriangles));
				ImGui::Separator();
				ImGui::Checkbox("Triangles Outline", &(model.trianglesOutlines));
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Light"))
		{
			if (ImGui::Button("Add Light")) {
				shared_ptr<Light>& newLight = std::make_shared<Light>();
				scene.AddLight(newLight);
			}
			// display transformations window if at least one model is loaded
			if (scene.GetLightCount()) {

				ImGui::Text("Select Light:");

				static int selectedLight = 0;
				for (int n = 0; n < scene.GetLightCount(); n++) {
					std::string camName = "Light ";
					camName.append(std::to_string(n));
					char bufc[64];
					sprintf(bufc, camName.c_str(), n);
					if (ImGui::Selectable(bufc, selectedLight == n))
						selectedLight = n;
				}

				Light& light = scene.GetLight(selectedLight);

				ImGui::Text("Local Translate:");
				ImGui::InputFloat("Local Translate X", &light.localTranslateArray[0], 0.1, 5, "%.2f");
				ImGui::InputFloat("Local Translate Y", &light.localTranslateArray[1], 0.1, 5, "%.2f");
				ImGui::InputFloat("Local Translate Z", &light.localTranslateArray[2], 0.1, 5, "%.2f");


				ImGui::InputFloat("high value1", &light.highValue1, 100, 5, "%.2f");
				ImGui::InputFloat("high value2", &light.highValue2, 100, 5, "%.2f");
				ImGui::InputFloat("high value3", &light.highValue3, 100, 5, "%.2f");

				ImGui::Text("Lighting Type:");
				ImGui::SameLine();
				ImGui::RadioButton("flat shading", &(light.lightingType), 0);
				ImGui::SameLine();
				ImGui::RadioButton("gouraud", &(light.lightingType), 1);
				ImGui::SameLine();
				ImGui::RadioButton("phong", &(light.lightingType), 2);
				
				ImGui::Separator();
				ImGui::InputFloat("alpha for specular", &(light.alpha), 0.1, 0.1, "%.2f");
				
				ImGui::Separator();
				ImGui::ColorEdit3("Light ambient Color", (float*)&light.ambientColor);

				ImGui::Separator();
				ImGui::ColorEdit3("Light diffuse Color", (float*)&light.diffuseColor);

				ImGui::Separator();
				ImGui::ColorEdit3("Light specular Color", (float*)&light.specularColor);



				ImGui::EndTabItem();
			}


			ImGui::EndTabBar();
		}
		ImGui::End();





	}

}