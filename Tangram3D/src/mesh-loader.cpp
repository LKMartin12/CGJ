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
  //std::vector<mgl::Mesh*> Meshes;
  //std::vector<glm::mat4> ModelMatrices;
  SceneGraph sceneGraph;
  bool rotatingView = false;
  double mouse_x, mouse_y;

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
  void rotateCamera(float angleX, float angleY);

  float animationProgress = 0.0f; // Progress of the animation (0.0 to 1.0)
  bool isAnimating = false;       // Is animation running 
  float animationSpeed = 0.5f;    // Speed of the animation
  bool isLeftKeyPressed = false;
  bool isRightKeyPressed = false;
  glm::mat4 startMatrix;
  glm::mat4 endMatrix;
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
  std::string mesh_dir = "assets/";
  std::string mesh_file = "square.obj";
  std::string mesh_file2 = "parallelogram.obj";
  std::string mesh_file3 = "triangle.obj";
  //std::string mesh_file = "monkey-torus-vtn-flat.obj";
  std::string mesh_fullname = mesh_dir + mesh_file;
  SquareMesh = new mgl::Mesh();
  SquareMesh->joinIdenticalVertices();
  SquareMesh->create(mesh_fullname);
  //Meshes.push_back(SquareMesh);
  sceneGraph.addNode(Node(SquareMesh, glm::mat4(1.0f)));
  sceneGraph.setNodeColor(0, glm::vec3(0.0f, 0.6f, 0.0f)); //square color (green)	

  mesh_fullname = mesh_dir + mesh_file2;
  ParallelogramMesh = new mgl::Mesh();
  ParallelogramMesh->joinIdenticalVertices();
  ParallelogramMesh->create(mesh_fullname);
  //Meshes.push_back(ParallelogramMesh);
  sceneGraph.addNode(Node(ParallelogramMesh, glm::mat4(1.0f)));
  sceneGraph.setNodeColor(1, glm::vec3(1.0f, 0.647f, 0.0f)); //paralelogram color (orange)
  

  mesh_fullname = mesh_dir + mesh_file3;
  
	  TriangleMesh = new mgl::Mesh();
	  TriangleMesh->joinIdenticalVertices();
	  TriangleMesh->create(mesh_fullname);
	  //Meshes.push_back(TriangleMesh);
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
  //ModelMatrices.resize(Meshes.size(), glm::mat4(1.0f));
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
	sceneGraph.createShaderPrograms();
}

///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(5.0f, 0.2f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(0.0f, 0.2f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10) 
//projection 0
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
//projection 1
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

std::vector<glm::mat4> figureModelMatrices = {
		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, 0.69f, -0.30f))*
		glm::rotate(glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::scale(glm::vec3(1.0f, 0.6f, 0.6f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, -0.38f, -0.1f))*
		glm::scale(glm::vec3(1.0f, 0.7f, 0.7f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, -0.58f, 0.1f))*
		glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::scale(glm::vec3(1.0f, 0.51f, 0.51f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, -0.51f, -0.25f))*
		glm::rotate(glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::scale(glm::vec3(1.0f, 0.5f, 0.5f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, 0.45f, -0.10f))*
		glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::scale(glm::vec3(1.0f, 0.73f, 0.73f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, 1.0f, 0.46f))*
		glm::rotate(glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f)),

		glm::mat4(1.0f)*
		glm::translate(glm::vec3(0.0f, 0.31f, 0.86f))*
		glm::rotate(glm::radians(150.0f), glm::vec3(1.0f, 0.0f, 0.0f))
};

std::vector<glm::mat4> boxModelMatrices = {
	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, -0.20f, 0.033f)) *
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 0.685f, 0.685f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, 0.09f, 0.33f)) *
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 0.7f, 0.7f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, 0.09f, 0.325f)) *
	glm::rotate(glm::radians(135.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 0.515f, 0.51f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, -0.49f, -0.265f)) *
	glm::rotate(glm::radians(225.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 0.515f, 0.515f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, -0.20f, 0.335f)) *
	glm::scale(glm::vec3(1.0f, 0.73f, 0.73f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, 0.695f, 0.035f)) *
	glm::rotate(glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 1.05f, 1.05f)),

	glm::mat4(1.0f) *
	glm::rotate(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * 
	glm::translate(glm::vec3(0.0f, 0.10f, -0.565f)) *
	glm::rotate(glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f)) *
	glm::scale(glm::vec3(1.0f, 1.05f, 1.05f))
};




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

//glm::mat4 ModelMatrix(1.0f);

glm::mat4 static interpolateMatrices(glm::mat4& start, glm::mat4& end, float t) {
	// Extract translation
	glm::vec3 startTranslation(start[3][0], start[3][1], start[3][2]);
	glm::vec3 endTranslation(end[3][0], end[3][1], end[3][2]);

	// Interpolate translation
	glm::vec3 interpolatedTranslation = glm::mix(startTranslation, endTranslation, t);

	// Interpolate rotation (normalize to ensure consistency)
	glm::quat startRotation = glm::normalize(glm::quat_cast(start));
	glm::quat endRotation = glm::normalize(glm::quat_cast(end));
	glm::quat interpolatedRotation = glm::slerp(startRotation, endRotation, t);

	// Interpolate scale (ensure uniform scaling)
	glm::vec3 startScale(
		glm::length(glm::vec3(start[0][0], start[1][0], start[2][0])),
		glm::length(glm::vec3(start[0][1], start[1][1], start[2][1])),
		glm::length(glm::vec3(start[0][2], start[1][2], start[2][2]))
	);
	glm::vec3 endScale(
		glm::length(glm::vec3(end[0][0], end[1][0], end[2][0])),
		glm::length(glm::vec3(end[0][1], end[1][1], end[2][1])),
		glm::length(glm::vec3(end[0][2], end[1][2], end[2][2]))
	);
	glm::vec3 interpolatedScale = glm::mix(startScale, endScale, t);

	// Combine the interpolated components into a single matrix
	glm::mat4 translationMatrix = glm::translate(interpolatedTranslation);
	glm::mat4 rotationMatrix = glm::mat4_cast(interpolatedRotation);
	glm::mat4 scaleMatrix = glm::scale(interpolatedScale);

	return translationMatrix * rotationMatrix * scaleMatrix;
}




void MyApp::drawScene() {
	if (!isAnimating) {
		// If animation is not active, set nodes directly based on progress
		for (size_t i = 0; i < figureModelMatrices.size(); ++i) {
			if (animationProgress <= 0.0f) {
				// At figure configuration
				sceneGraph.nodes[i].modelMatrix = figureModelMatrices[i];
			}
			else if (animationProgress >= 1.0f) {
				// At box configuration
				sceneGraph.nodes[i].modelMatrix = boxModelMatrices[i];
			}
		}
	}
	else {

		if (isLeftKeyPressed) {
			float delta = animationSpeed * 0.016f; // Assuming ~60 FPS
			animationProgress += delta;
			if (animationProgress >= 1.0f) {
				animationProgress = 1.0f;
				isAnimating = false; // Stop animation
			}
		}
		else if (isRightKeyPressed) {
			float delta = animationSpeed * 0.016f; // Assuming ~60 FPS
			animationProgress -= delta;
			if (animationProgress <= 0.0f) {
				animationProgress = 0.0f;
				isAnimating = false; // Stop animation
			}
		}
		//std::cout << "AnimationProgress: " << animationProgress << std::endl;

		// Interpolate model matrices based on animationProgress
		for (size_t i = 0; i < figureModelMatrices.size(); ++i) {
			if (isLeftKeyPressed) {
				//std::cout << "Vai para a caixa" << std::endl;
				startMatrix = figureModelMatrices[i];
				endMatrix = boxModelMatrices[i];
				sceneGraph.nodes[i].modelMatrix = interpolateMatrices(startMatrix, endMatrix, animationProgress);
			}
			else if (isRightKeyPressed) {
				//std::cout << "Vai para a figura" << std::endl;
				startMatrix = boxModelMatrices[i];
				endMatrix = figureModelMatrices[i];
				sceneGraph.nodes[i].modelMatrix = interpolateMatrices(startMatrix, endMatrix, 1 - animationProgress);
			}
			else {
				sceneGraph.nodes[i].modelMatrix = CurrentModelMatrix[i];
			}
			CurrentModelMatrix[i] = sceneGraph.nodes[i].modelMatrix;
		}
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
}

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
  // change projection matrices to maintain aspect ratio

  float aspectRatio = static_cast<float>(winx) / static_cast<float>(winy);

  if (CurrentProjectionMatrix1 == Camera->getProjectionMatrix()) {
	  std::cout << "Orthographic resize" << std::endl;
	  CurrentProjectionMatrix1 = glm::ortho(-2.0f*aspectRatio, 2.0f*aspectRatio, -2.0f*aspectRatio, 2.0f*aspectRatio, 1.0f*aspectRatio, 10.0f*aspectRatio);
	  Camera->setProjectionMatrix(CurrentProjectionMatrix1);
  }
  else {
	  std::cout << "Perspective resize" << std::endl;
	  CurrentProjectionMatrix2 = glm::perspective(glm::radians(30.0f), aspectRatio, 1.0f, 10.0f);
	  Camera->setProjectionMatrix(CurrentProjectionMatrix2);
  }

  
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { drawScene(); }

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_P: // Switch projection of the current camera
			if (Camera->getProjectionMatrix() == ProjectionMatrix1) {
				Camera->setProjectionMatrix(ProjectionMatrix2);
			}
			else {
				Camera->setProjectionMatrix(ProjectionMatrix1);
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
				isAnimating = true; // Enable animation
			}
			break;

		case GLFW_KEY_RIGHT: // Start animation towards the figure
			if (animationProgress != 0.0f && !isLeftKeyPressed) {
				isRightKeyPressed = true;
				isAnimating = true; // Enable animation
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
	// Zoom factor: adjust this value for smoother zooming
	const float zoomSpeed = 0.5f;

	// Get the current view matrix and camera position
	glm::mat4 viewMatrix = Camera->getViewMatrix();
	glm::vec3 cameraPosition = glm::vec3(glm::inverse(viewMatrix)[3]);
	glm::vec3 center(0.0f, 0.0f, 0.0f);  // Center of the scene

	// Calculate the direction vector
	glm::vec3 direction = glm::normalize(center - cameraPosition);

	// Adjust the camera position based on scroll input
	cameraPosition += static_cast<float>(yoffset) * zoomSpeed * direction;

	if (CurrentViewMatrix1 == Camera->getViewMatrix()) {
		// Update the view matrix
		Camera->setViewMatrix(glm::lookAt(cameraPosition, center, glm::vec3(0.0f, 1.0f, 0.0f)));
		CurrentViewMatrix1 = Camera->getViewMatrix();
	}
	else {
		Camera->setViewMatrix(glm::lookAt(cameraPosition, center, glm::vec3(0.0f, 1.0f, 0.0f)));
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
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::quat q_x = glm::angleAxis(angleX, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 camDirection = glm::normalize(center - cameraPosition);
	glm::vec3 cam_right = glm::normalize(glm::cross(camDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::quat q_y = glm::angleAxis(angleY, cam_right);
	glm::quat q = q_x * q_y;

	cameraPosition = q * cameraPosition;
	Camera->setViewMatrix(glm::lookAt(cameraPosition, center, glm::vec3(0.0f, 1.0f, 0.0f)));
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
