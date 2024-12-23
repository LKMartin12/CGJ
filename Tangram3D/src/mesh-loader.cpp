////////////////////////////////////////////////////////////////////////////////
//
//  Loading meshes from external files
//
// Copyright (c) 2023-24 by Carlos Martinho
//
// INTRODUCES:
// MODEL DATA, ASSIMP, mglMesh.hpp
//
////////////////////////////////////////////////////////////////////////////////

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <iostream>
#include "mgl/mgl.hpp"

////////////////////////////////////////////////////////////////////////// MYAPP

class Node {
public:
	mgl::Mesh* mesh;
	mgl::ShaderProgram* shader;
	glm::mat4 modelMatrix;
	GLint modelMatrixId;
	GLint colorId;
	glm::vec3 color;

	Node(mgl::Mesh* mesh, const glm::mat4& modelMatrix)
		: mesh(mesh), shader(shader), modelMatrix(modelMatrix) {}

	// Create and configure the shader
	void createShaderProgram() {
		shader = new mgl::ShaderProgram();
		shader->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
		shader->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

		shader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
		shader->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);

		shader->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);

		shader->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);

		shader->addUniform(mgl::MODEL_MATRIX);
		shader->addUniform("givenColor");
		shader->addUniformBlock(mgl::CAMERA_BLOCK, 0);
		shader->create();

		colorId = shader->Uniforms["givenColor"].index;
		modelMatrixId = shader->Uniforms[mgl::MODEL_MATRIX].index;
	}

	// Transformations for the node
	void translate(const glm::vec3& translation) {
		modelMatrix = glm::translate(modelMatrix, translation);
	}

	void rotate(float angle, const glm::vec3& axis) {
		modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
	}

	void scale(const glm::vec3& scale) {
		modelMatrix = glm::scale(modelMatrix, scale);
	}

	void resetModelMatrix() {
		modelMatrix = glm::mat4(1.0f);
	}

	void setColor(const glm::vec3& color) {
		this->color = color;
	}
};

class SceneGraph {
public:
	std::vector<Node> nodes;

	void addNode(const Node& node) {
		nodes.push_back(node);
	}

	void draw() {
		for (const auto& node : nodes) {
			node.shader->bind();
			glUniform3fv(node.colorId, 1, glm::value_ptr(node.color));
			glUniformMatrix4fv(node.modelMatrixId, 1, GL_FALSE, glm::value_ptr(node.modelMatrix));
			node.mesh->draw();
			node.shader->unbind();
		}
	}

	void createShaderPrograms() {
		for (auto& node : nodes) {
			node.createShaderProgram();
		}
	}
	void resetNodesTransformations() {
		for (auto& node : nodes) {
			node.resetModelMatrix();
		}
	}

	void setNodeColor(int index ,const glm::vec3& color) {
		this->nodes[index].setColor(color);
	}

};

class MyApp : public mgl::App {
 public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;
  void scrollCallback(GLFWwindow* win, double xoffset, double yoffset);
  void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) override;
  void cursorCallback(GLFWwindow* win, double xpos, double ypos) override;

 private:
  const GLuint UBO_BP = 0;
  mgl::ShaderProgram *Shaders = nullptr;
  mgl::Camera *Camera = nullptr;
  GLint ModelMatrixId;
  mgl::Mesh* SquareMesh = nullptr;
  mgl::Mesh* TriangleMesh = nullptr;
  mgl::Mesh* ParallelogramMesh = nullptr;
  SceneGraph sceneGraph;
  bool rotatingView = false;
  double mouse_x, mouse_y;

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
  void rotateCamera(float angleX, float angleY);


  // Animation variables for transitioning between two configurations
  float animationProgress = 0.0f;
  float animationSpeed = 0.5f;    
  bool isLeftKeyPressed = false;
  bool isRightKeyPressed = false;
  glm::mat4 startMatrix;
  glm::mat4 endMatrix;
  float previousTime = 0.0f;
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
  std::string mesh_dir = "assets/";
  std::string mesh_file = "square.obj";
  std::string mesh_file2 = "parallelogram.obj";
  std::string mesh_file3 = "triangle.obj";

  std::string mesh_fullname = mesh_dir + mesh_file;

  // Load square mesh
  SquareMesh = new mgl::Mesh();
  SquareMesh->joinIdenticalVertices();
  SquareMesh->create(mesh_fullname);
  sceneGraph.addNode(Node(SquareMesh, glm::mat4(1.0f)));
  sceneGraph.setNodeColor(0, glm::vec3(0.0f, 0.6f, 0.0f)); //square color (green)	

  mesh_fullname = mesh_dir + mesh_file2;

  // Load parallelogram mesh
  ParallelogramMesh = new mgl::Mesh();
  ParallelogramMesh->joinIdenticalVertices();
  ParallelogramMesh->create(mesh_fullname);
  sceneGraph.addNode(Node(ParallelogramMesh, glm::mat4(1.0f)));
  sceneGraph.setNodeColor(1, glm::vec3(1.0f, 0.647f, 0.0f)); //paralelogram color (orange)
  

  mesh_fullname = mesh_dir + mesh_file3;
  
  // Load triangle mesh and add several nodes of the same mesh with different colors
  TriangleMesh = new mgl::Mesh();
  TriangleMesh->joinIdenticalVertices();
  TriangleMesh->create(mesh_fullname);

  for (uint16_t i = 0; i < 5; i++) {
	sceneGraph.addNode(Node(TriangleMesh, glm::mat4(1.0f)));
	if (i == 0) {
		sceneGraph.setNodeColor(i + 2, glm::vec3(0.376f, 0.482f, 0.745f)); //small triangle 1 color (greyed-blue)
	}
	else if (i == 1) {
		sceneGraph.setNodeColor(i + 2, glm::vec3(1.000, 0.271, 0.0)); //small triangle 2 color (orange-red)
	}
	else if (i == 2) {
		sceneGraph.setNodeColor(i + 2, glm::vec3(0.502f, 0.0f, 0.502f)); //mid-size triangle color (purple)
	}
	else if (i == 3) {
		sceneGraph.setNodeColor(i + 2, glm::vec3(0.275f, 0.460f, 0.806f)); //big triangle 1 color (blue)
	}
	else if (i == 4) {
		sceneGraph.setNodeColor(i + 2, glm::vec3(0.780f, 0.082f, 0.522f)); //big triangle 2 color (pink-red)
	}
  }
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
	sceneGraph.createShaderPrograms();
}

///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5, 0.2, 0.1)  Center(0, 0, 0)  Up(0, 1.0, 0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(5.0f, 0.2f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(0.0, 0.2, 5.0)  Center(0, 0, 0)  Up(0, 1.0, 0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(0.0f, 0.2f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic projection (ProjectionMatrix1)
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f*(640.0f/480.0f), 2.0f*(640.0f/480.0f), -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective projection (ProjectionMatrix2)
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);


// These model matrices represent two different configurations (figure and box).
// figureModelMatrices: positions/orientations for the "figure" configuration
// boxModelMatrices: positions/orientations for the "box" configuration

std::vector<glm::mat4> figureModelMatrices = {

	// square
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, 0.69f, -0.30f))*
	glm::rotate(glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.6f, 0.6f)),

	// parallelogram
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, -0.38f, -0.1f))*
	glm::scale(glm::vec3(1.0f, 0.7f, 0.7f)),

	// small triangle 1
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, -0.58f, 0.1f))*
	glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.51f, 0.51f)),

	// small triangle 2
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, -0.51f, -0.25f))*
	glm::rotate(glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.5f, 0.5f)),

	// mid-size triangle
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, 0.45f, -0.10f))*
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.73f, 0.73f)),

	// big triangle 1
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, 1.0f, 0.46f))*
	glm::rotate(glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f)),

	// big triangle 2
	glm::mat4(1.0f)*
	glm::translate(glm::vec3(0.0f, 0.31f, 0.86f))*
	glm::rotate(glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f))

};

std::vector<glm::mat4> boxModelMatrices = {

	// square
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, -0.195f, 0.033f))*
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.695f, 0.695f)),

	// parallelogram
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, 0.095f, 0.325f))*
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.7f, 0.7f)),

	// small triangle 1
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, 0.1f, 0.325f))*
	glm::rotate(glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.515f, 0.51f)),

	// small triangle 2
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, -0.49f, -0.265f))*
	glm::rotate(glm::radians(225.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 0.515f, 0.515f)),

	// mid-size triangle
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, -0.20f, 0.325f))*
	glm::scale(glm::vec3(1.0f, 0.73f, 0.73f)),

	// big triangle 1
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, 0.695f, 0.035f))*
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 1.05f, 1.05f)),

	// big triangle 2
	glm::mat4(1.0f)*
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f))*
	glm::translate(glm::vec3(0.0f, 0.10f, -0.565f))*
	glm::rotate(glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
	glm::scale(glm::vec3(1.0f, 1.05f, 1.05f))
};


// Variables that help saving stages of the animation process

glm::mat4 CurrentViewMatrix1 = ViewMatrix1;
glm::mat4 CurrentViewMatrix2 = ViewMatrix2;
int CurrentCam = 1;
glm::mat4 CurrentProjectionMatrix1 = ProjectionMatrix1;
glm::mat4 CurrentProjectionMatrix2 = ProjectionMatrix2;
std::vector<glm::mat4> CurrentModelMatrix;

void MyApp::createCamera() {
  Camera = new mgl::Camera(UBO_BP);
  Camera->setViewMatrix(ViewMatrix1);
  Camera->setProjectionMatrix(CurrentProjectionMatrix2);
}

/////////////////////////////////////////////////////////////////////////// DRAW


// Helper function to interpolate between two models

glm::mat4 interpolateMatrices(const glm::mat4& start, const glm::mat4& end, float t) {
	glm::vec3 startTranslation, startScale, startSkew;
	glm::quat startRotation;
	glm::vec4 startPerspective;
	glm::decompose(start, startScale, startRotation, startTranslation, startSkew, startPerspective);

	glm::vec3 endTranslation, endScale, endSkew;
	glm::quat endRotation;
	glm::vec4 endPerspective;
	glm::decompose(end, endScale, endRotation, endTranslation, endSkew, endPerspective);

	glm::vec3 interpTranslation = glm::mix(startTranslation, endTranslation, t);
	glm::vec3 interpScale = glm::mix(startScale, endScale, t);
	glm::quat interpRotation = glm::slerp(startRotation, endRotation, t);

	return glm::translate(interpTranslation) * glm::mat4_cast(interpRotation) * glm::scale(interpScale);
}



void MyApp::drawScene() {

	double currentTime = glfwGetTime();

	//this ensures that the animation speed is constant between computers
	float deltaTime = static_cast<float>(currentTime - previousTime);

	previousTime = currentTime;

	// Update animation progress if left or right keys are pressed
	if (isLeftKeyPressed) {
		float delta = animationSpeed * deltaTime;
		animationProgress += delta;
		if (animationProgress >= 1.0f) {
			animationProgress = 1.0f;
		}
	}
	else if (isRightKeyPressed) {
		float delta = animationSpeed * deltaTime; // 
		animationProgress -= delta;
		if (animationProgress <= 0.0f) {
			animationProgress = 0.0f;
		}
	}
	
	// Interpolate model matrices based on animationProgress
	for (size_t i = 0; i < figureModelMatrices.size(); i++) {
		if (isLeftKeyPressed || animationProgress == 0.0f) {
			startMatrix = figureModelMatrices[i];
			endMatrix = boxModelMatrices[i];
			sceneGraph.nodes[i].modelMatrix = interpolateMatrices(startMatrix, endMatrix, animationProgress);
		}
		else if (isRightKeyPressed || animationProgress == 1.0f) {
			startMatrix = boxModelMatrices[i];
			endMatrix = figureModelMatrices[i];
			sceneGraph.nodes[i].modelMatrix = interpolateMatrices(startMatrix, endMatrix, 1 - animationProgress);
		}
		else {
			sceneGraph.nodes[i].modelMatrix = CurrentModelMatrix[i];
		}
		CurrentModelMatrix[i] = sceneGraph.nodes[i].modelMatrix;
	}
	sceneGraph.draw();
	sceneGraph.resetNodesTransformations();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
	createMeshes();
	createShaderPrograms();
	createCamera();

	CurrentModelMatrix.resize(figureModelMatrices.size(), glm::mat4(1.0f));

	previousTime = glfwGetTime();
}

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);

  // Recompute projection matrices to maintain aspect ratio
  float aspectRatio = static_cast<float>(winx) / static_cast<float>(winy);

  glm::mat4 updatedProjectionMatrix1 = glm::ortho(-2.0f * aspectRatio, 2.0f * aspectRatio, -2.0f, 2.0f, 1.0f, 10.0f);
  glm::mat4 updatedProjectionMatrix2 = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 10.0f);

  if (CurrentProjectionMatrix1 == Camera->getProjectionMatrix()) {
	  CurrentProjectionMatrix1 = updatedProjectionMatrix1;
	  CurrentProjectionMatrix2 = updatedProjectionMatrix2;
	  Camera->setProjectionMatrix(CurrentProjectionMatrix1);
  }
  else {
	  CurrentProjectionMatrix2 = updatedProjectionMatrix2;
	  CurrentProjectionMatrix1 = updatedProjectionMatrix1;
	  Camera->setProjectionMatrix(CurrentProjectionMatrix2);
  }

  
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { drawScene(); }

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_P: // Switch projection of the current camera
			if (Camera->getProjectionMatrix() == CurrentProjectionMatrix1) {
				Camera->setProjectionMatrix(CurrentProjectionMatrix2);
			}
			else {
				Camera->setProjectionMatrix(CurrentProjectionMatrix1);
			}
			break;

		case GLFW_KEY_C: // Switch camera (view)
			if (CurrentCam == 1) {
				Camera->setViewMatrix(CurrentViewMatrix2);
				CurrentCam = 0;
			}
			else {
				Camera->setViewMatrix(CurrentViewMatrix1);
				CurrentCam = 1;
			}
			break;

		case GLFW_KEY_LEFT: // Start animation towards the box
			if (animationProgress != 1.0f && !isRightKeyPressed) {
				isLeftKeyPressed = true;
			}
			break;

		case GLFW_KEY_RIGHT: // Start animation towards the figure
			if (animationProgress != 0.0f && !isLeftKeyPressed) {
				isRightKeyPressed = true;
			}
			break;
		}
		break;

	case GLFW_RELEASE:
		switch (key) {
		case GLFW_KEY_LEFT:
			isLeftKeyPressed = false; // Stop tracking the left key
			break;

		case GLFW_KEY_RIGHT:
			isRightKeyPressed = false; // Stop tracking the right key
			break;
		}
		break;
	}
}


void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {

	const float zoomSpeed = 0.1f;

	// Get the current view matrix and camera position
	glm::mat4 viewMatrix = Camera->getViewMatrix();
	glm::vec3 cameraPosition = glm::vec3(glm::inverse(viewMatrix)[3]);
	glm::vec3 upVector = glm::vec3(glm::inverse(viewMatrix)[1]);
	glm::vec3 center(0.0f, 0.0f, 0.0f);  // Center of the scene

	// Calculate the direction vector
	glm::vec3 direction = glm::normalize(center - cameraPosition);

	// Adjust the camera position based on scroll input
	cameraPosition += static_cast<float>(yoffset) * zoomSpeed * direction;

	if (CurrentViewMatrix1 == Camera->getViewMatrix()) {
		Camera->setViewMatrix(glm::lookAt(cameraPosition, center, upVector));
		CurrentViewMatrix1 = Camera->getViewMatrix();
	}
	else {
		Camera->setViewMatrix(glm::lookAt(cameraPosition, center, upVector));
		CurrentViewMatrix2 = Camera->getViewMatrix();
	}

}

void MyApp::mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetCursorPos(win, &mouse_x, &mouse_y);
		rotatingView = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		rotatingView = false;
	}
	
}

void MyApp::rotateCamera(float angleX, float angleY) {
	glm::mat4 cameraView = Camera->getViewMatrix();
	glm::vec3 cameraPosition = glm::vec3(glm::inverse(cameraView)[3]);
	glm::vec3 cameraUp = glm::vec3(glm::inverse(cameraView)[1]);
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);

	// Rotate around up and right vectors to achieve orbiting
	glm::quat q_x = glm::angleAxis(angleX, cameraUp);
	glm::vec3 camDirection = glm::normalize(center - cameraPosition);
	glm::vec3 cam_right = glm::normalize(glm::cross(camDirection, cameraUp));
	glm::quat q_y = glm::angleAxis(angleY, cam_right);
	glm::quat q = q_x * q_y;

	cameraUp = q * cameraUp;
	cameraPosition = q * cameraPosition;
	Camera->setViewMatrix(glm::lookAt(cameraPosition, center, cameraUp));
	if (CurrentCam == 1) {
		CurrentViewMatrix1 = Camera->getViewMatrix();
	}
	else {
		CurrentViewMatrix2 = Camera->getViewMatrix();
	}
}

void MyApp::cursorCallback(GLFWwindow* win, double xpos, double ypos) {
	if (!rotatingView) return;

	const double sensitivity = 0.005f;
	double dx = xpos - mouse_x;
	double dy = ypos - mouse_y;

	mouse_x = xpos;
	mouse_y = ypos;

	float angleX = static_cast<float>(-dx * sensitivity) ;
	float angleY = static_cast<float>(-dy * sensitivity) ;

	rotateCamera(angleX, angleY);

}


/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "Tangram 3D Group 11", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
