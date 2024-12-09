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
		shader->addUniformBlock(mgl::CAMERA_BLOCK, 0);
		shader->create();

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

};

class MyApp : public mgl::App {
 public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;
  void scrollCallback(GLFWwindow* win, double xoffset, double yoffset);

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

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
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

  mesh_fullname = mesh_dir + mesh_file2;
  ParallelogramMesh = new mgl::Mesh();
  ParallelogramMesh->joinIdenticalVertices();
  ParallelogramMesh->create(mesh_fullname);
  //Meshes.push_back(ParallelogramMesh);
  sceneGraph.addNode(Node(ParallelogramMesh, glm::mat4(1.0f)));

  mesh_fullname = mesh_dir + mesh_file3;
  
	  TriangleMesh = new mgl::Mesh();
	  TriangleMesh->joinIdenticalVertices();
	  TriangleMesh->create(mesh_fullname);
	  //Meshes.push_back(TriangleMesh);
  for (uint16_t i = 0; i < 5; i++) {
	sceneGraph.addNode(Node(TriangleMesh, glm::mat4(1.0f)));
  }
  //ModelMatrices.resize(Meshes.size(), glm::mat4(1.0f));
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
  /*Shaders = new mgl::ShaderProgram();
  Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
  Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
  
  Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
  
  
  Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
  

  Shaders->addUniform(mgl::MODEL_MATRIX);
  Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
  Shaders->create();

  ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;*/

	sceneGraph.createShaderPrograms();
}

///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(5.0f, 0.2f, 0.1f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(5.0f, 0.2f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10) 
//projection 0
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
//projection 1
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

glm::mat4 CurrentViewMatrix1 = ViewMatrix1;
glm::mat4 CurrentViewMatrix2 = ViewMatrix2;
int CurrentCam = 1;
glm::mat4 CurrentProjectionMatrix1 = ProjectionMatrix2;
glm::mat4 CurrentProjectionMatrix2 = ProjectionMatrix2;

void MyApp::createCamera() {
  Camera = new mgl::Camera(UBO_BP);
  Camera->setViewMatrix(ViewMatrix1);
  Camera->setProjectionMatrix(ProjectionMatrix2);
}

/////////////////////////////////////////////////////////////////////////// DRAW

//glm::mat4 ModelMatrix(1.0f);

void MyApp::drawScene() {
	/*Shaders->bind();
	for (uint16_t i = 0; i < 7; i++) {
		glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrices[i]));
		Meshes[i]->draw();
	}
	Shaders->unbind();*/
	
	//sceneGraph.nodes[2].translate(glm::vec3(0.0f, 0.5f, 0.0f));
	sceneGraph.nodes[0].translate(glm::vec3(0.0f, 0.69f, -0.30f));
	sceneGraph.nodes[0].rotate(20.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[0].scale(glm::vec3(1.0f, 0.6f, 0.6f));
	sceneGraph.nodes[1].translate(glm::vec3(0.0f, -0.38f, -0.1f));
	sceneGraph.nodes[1].scale(glm::vec3(1.0f, 0.7f, 0.7f));
	sceneGraph.nodes[2].translate(glm::vec3(0.0f, -0.58f, 0.1f));
	sceneGraph.nodes[2].rotate(90.0f ,glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[2].scale(glm::vec3(1.0f, 0.5f, 0.5f));
	sceneGraph.nodes[3].translate(glm::vec3(0.0f, -0.51f, -0.25f));
	sceneGraph.nodes[3].rotate(135.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[3].scale(glm::vec3(1.0f, 0.5f, 0.5f));
	sceneGraph.nodes[4].translate(glm::vec3(0.0f, 0.45f, -0.10f));
	sceneGraph.nodes[4].rotate(45.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[4].scale(glm::vec3(1.0f, 0.73f, 0.73f));
	sceneGraph.nodes[5].translate(glm::vec3(0.0f, 1.0f, 0.46f));
	sceneGraph.nodes[5].rotate(60.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[5].scale(glm::vec3(1.0f, 1.0f, 1.0f));
	sceneGraph.nodes[6].translate(glm::vec3(0.0f, 0.31f, 0.86f));
	sceneGraph.nodes[6].rotate(150.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	sceneGraph.nodes[6].scale(glm::vec3(1.0f, 1.0f, 1.0f));
	sceneGraph.draw();
	sceneGraph.resetNodesTransformations();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
	createMeshes();
	createShaderPrograms();
	createCamera();
}

void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
  glViewport(0, 0, winx, winy);
  // change projection matrices to maintain aspect ratio
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { drawScene(); }

void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
	/*if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(win, GLFW_TRUE);
	}*/
	//std::cout << "key: " << key << " scancode: " << scancode << " action: " << action << " mods: " << mods << std::endl;
	switch (action){
	case 1:
		switch (key){
		case 80: //P
			//switch projection of current camera
			if (Camera->getProjectionMatrix() == ProjectionMatrix1) {
				Camera->setProjectionMatrix(ProjectionMatrix2);
			}
			else {
				Camera->setProjectionMatrix(ProjectionMatrix1);
			}
			break;
		case 67: //C
			//switch camera (view)
			if (CurrentCam==1) {
				Camera->setViewMatrix(CurrentViewMatrix2);
				//Camera->setProjectionMatrix(CurrentProjectionMatrix2);
				CurrentCam = 0;
			}
			else {
				Camera->setViewMatrix(CurrentViewMatrix1);
				//Camera->setProjectionMatrix(CurrentProjectionMatrix1);
				CurrentCam = 1;
			}
			break;
		case 262: //right arrow key
			//animate the meshes to one configuration
			break;
		case 263: //left arrow key
			//animate the meshes to other configuration
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
