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

class MyApp : public mgl::App {
 public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;

 private:
  const GLuint UBO_BP = 0;
  mgl::ShaderProgram *Shaders = nullptr;
  mgl::Camera *Camera = nullptr;
  GLint ModelMatrixId;
  mgl::Mesh* SquareMesh = nullptr;
  mgl::Mesh* TriangleMesh = nullptr;
  mgl::Mesh* ParallelogramMesh = nullptr;
  std::vector<mgl::Mesh*> Meshes;
  std::vector<glm::mat4> ModelMatrices;

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
};

///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
  std::string mesh_dir = "assets/";
  // std::string mesh_file = "cube-v.obj";
  // std::string mesh_file = "cube-vn-flat.obj";
  // std::string mesh_file = "cube-vn-smooth.obj";
  // std::string mesh_file = "cube-vt.obj";
  // std::string mesh_file = "cube-vt2.obj";
  // std::string mesh_file = "torus-vtn-flat.obj";
  // std::string mesh_file = "torus-vtn-smooth.obj";
  // std::string mesh_file = "suzanne-vtn-flat.obj";
  // std::string mesh_file = "suzanne-vtn-smooth.obj";
  // std::string mesh_file = "teapot-vn-flat.obj";
  // std::string mesh_file = "teapot-vn-smooth.obj";
  // std::string mesh_file = "bunny-vn-flat.obj";
  std::string mesh_file = "square.obj";
  std::string mesh_file2 = "parallelogram.obj";
  std::string mesh_file3 = "triangle.obj";
  //std::string mesh_file = "monkey-torus-vtn-flat.obj";
  std::string mesh_fullname = mesh_dir + mesh_file;
  SquareMesh = new mgl::Mesh();
  SquareMesh->joinIdenticalVertices();
  SquareMesh->create(mesh_fullname);
  Meshes.push_back(SquareMesh);

  mesh_fullname = mesh_dir + mesh_file2;
  ParallelogramMesh = new mgl::Mesh();
  ParallelogramMesh->joinIdenticalVertices();
  ParallelogramMesh->create(mesh_fullname);
  Meshes.push_back(ParallelogramMesh);

  mesh_fullname = mesh_dir + mesh_file3;
  for (uint16_t i = 0; i < 5; i++) {
	  //ModelMatrices.push_back(glm::translate(glm::vec3(i, 0, 0)));
	  TriangleMesh = new mgl::Mesh();
	  TriangleMesh->joinIdenticalVertices();
	  TriangleMesh->create(mesh_fullname);
	  Meshes.push_back(TriangleMesh);
  }
  ModelMatrices.resize(Meshes.size(), glm::mat4(1.0f));
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
  Shaders = new mgl::ShaderProgram();
  Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

  Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
  if (SquareMesh->hasNormals()) {
    Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
  }
  if (SquareMesh->hasTexcoords()) {
    Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
  }
  if (SquareMesh->hasTangentsAndBitangents()) {
    Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
  }

  Shaders->addUniform(mgl::MODEL_MATRIX);
  Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
  Shaders->create();

  ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}

///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(0.0f, 5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10) 
//projection 0
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
//projection 1
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

void MyApp::createCamera() {
  Camera = new mgl::Camera(UBO_BP);
  Camera->setViewMatrix(ViewMatrix1);
  Camera->setProjectionMatrix(ProjectionMatrix2);
}

/////////////////////////////////////////////////////////////////////////// DRAW

glm::mat4 ModelMatrix(1.0f);

void MyApp::drawScene() {
  Shaders->bind();
  for (uint16_t i = 0; i < 7; i++) {
	  /*if (i == 3) {
		  ModelMatrices[i] = glm::translate(glm::vec3(0, 0, 0));
	  }
	  else if (i == 4) {
		  ModelMatrices[i] = glm::translate(glm::vec3(1, 0, 0));
	  }
	  else if (i == 5) {
		  ModelMatrices[i] = glm::translate(glm::vec3(2, 0, 0));
	  }
	  else if (i == 6) {
		  ModelMatrices[i] = glm::translate(glm::vec3(3, 0, 0));
	  }
	  else {
		  ModelMatrices[i] = glm::translate(glm::vec3(4, 0, 0));
	  }*/
	 glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrices[i]));
	 Meshes[i]->draw();
  }
  
 /* SquareMesh->draw();
  TriangleMesh->draw();
  ParallelogramMesh->draw();*/

  Shaders->unbind();
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow *win) {
  createMeshes();
  createShaderPrograms();  // after mesh;
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
			if (Camera->getViewMatrix() == ViewMatrix1) {
				Camera->setViewMatrix(ViewMatrix2);
			}
			else {
				Camera->setViewMatrix(ViewMatrix1);
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

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "Mesh Loader", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
