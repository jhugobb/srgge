#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "Scene.h"
#include "PLYReader.h"
#include "LOD.h"

Scene::Scene()
{
	map = new Map();
}

Scene::~Scene()
{
	meshes.clear();
}

void parse(Map* map, string filename) {
  std::ifstream file(filename, ios::in);
  std::string line; 
  unsigned int size;
  unsigned int i = 0;
  while (std::getline(file, line)) {
    size = line.size(); 
    map->addRow(size);
    for (int j= 0; j< size; j++) {
      switch (line[j]) {
        case 'w':
          map->addWall(i,j);
          break;
        case 'f':
          map->addFloor(i,j);
          break;
				case 'b':
					map->addBunny(i,j);
					break;
        // TODO: add more cases;
        default:
          break;
      }
    }
    i++;
  }
	file.close();
}

void Scene::init()
{
	initShaders();
	parse(map, "../2.map");
	
	for (char c : map->getTypesOfMesh()) {
		TriangleMesh* mesh = new TriangleMesh();
		std::cout << "bah" << std::endl;
		bool success;
		LOD lod;
		PLYReader reader;
		switch (c) {
			case 'w':
				mesh->buildCube();
				mesh->sendToOpenGL(basicProgram);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				break;
			case 'f':
				mesh->buildFloor();
				mesh->sendToOpenGL(basicProgram);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				break;
			case 'b':
				success = reader.readMesh("../models/Armadillo.ply", *mesh);
				lod = LOD(mesh);
				assert(lod.simp_vertices.size() != 0);
				mesh->setLOD(lod);
				mesh->setLODlevel(1);
				if (success) mesh->sendToOpenGL(basicProgram);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				break;
			default:
				break;
		}
	}
	currentTime = 0.0f;
	
	camera.init(2.0f);
	
	bPolygonFill = true;
	
	// Select which font you want to use
	if(!text.init("fonts/OpenSans-Regular.ttf"))
	//if(!text.init("fonts/OpenSans-Bold.ttf"))
	//if(!text.init("fonts/DroidSerif.ttf"))
		cout << "Could not load font!!!" << endl;
}

bool Scene::loadMesh(const char *filename)
{
	PLYReader reader;

	//mesh->free();
	//bool bSuccess = reader.readMesh(filename, *mesh);
	//if(bSuccess)
	  //mesh->sendToOpenGL(basicProgram);
	
	//return bSuccess;
	return false;
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right) 
{
	currentTime += deltaTime;
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	framerate = 1.0f/deltaTime * 1000;
	glm::vec3 mov(0);
	float rate = float(1.0f/deltaTime);
	if (forward){
		mov.z += rate;
	} 
	if (back){
		mov.z -= rate;
	}
	if (left){
		mov.x += rate;
	} 
	if (right){
		mov.x -= rate;
	} 

	camera.move(mov);
}

void Scene::render()
{
	glm::mat3 normalMatrix;

	basicProgram.use();
	glm::mat4 MV = camera.getModelViewMatrix();
	basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
	normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	
	basicProgram.setUniform1i("bLighting", bPolygonFill?1:0);
	if(bPolygonFill)
	{
  	basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else
  {
  	basicProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.5f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		int size = map->layout.size();
		for (int i = 0; i<size; i++) {
			int s = map->layout[i].size();
			for (int j = 0; j<s; j++) {
				TriangleMesh* m = meshes[map->layout[i][j]];
				if(m) {
					glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
					glm::mat4 mat = MV* matrix;
					basicProgram.setUniformMatrix4f("modelview", mat);
					m->render();
				}
			}
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
  	basicProgram.setUniform4f("color", 0.0f, 0.0f, 0.0f, 1.0f);
  }
	int size = map->layout.size();
	for (int i = 0; i<size; i++) {
		int s = map->layout[i].size();
		for (int j = 0; j<s; j++) {
			TriangleMesh* m = meshes[map->layout[i][j]];
			if(m) {
				glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
				glm::mat4 mat = MV * matrix;
				basicProgram.setUniformMatrix4f("modelview", mat);
				m->render();
			}
			
		}
	}
	text.render(std::to_string(framerate), glm::vec2(20, 20), 16, glm::vec4(0, 0, 0, 1));
}

Camera &Scene::getCamera()
{
  return camera;
}

void Scene::switchPolygonMode()
{
  bPolygonFill = !bPolygonFill;
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::cleanup() {
	int size = map->layout.size();
	for (int i = 0; i<size; i++) {
		int s = map->layout[i].size();
		for (int j = 0; j<s; j++) {
			TriangleMesh* m = meshes[map->layout[i][j]];
			if(m)
				m->free();
		}
	}
}