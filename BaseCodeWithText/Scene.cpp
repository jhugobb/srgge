#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <ctime>
#include <iostream>

#include "Scene.h"
#include "PLYReader.h"
#include "LOD.h"

#define GLM_FORCE_RADIANS

Scene::Scene()
{
	map = new Map();
	tcr = TCR();
	delta = 0;
	framerate = 0;
}

Scene::~Scene()
{
	meshes.clear();
}

std::vector<std::string> split(const std::string & str, const std::string & delim) {
    std::vector<std::string> tokens;
    unsigned int prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

bool Scene::parseVisibility(string filename) {
	std::ifstream file(filename, ios::in);
	if (!file.is_open()) {
		cout << "Visibility file not found" << endl;
		return false;
	}
  std::string line; 
  unsigned int i = 0;
  while (std::getline(file, line)) {
		std::vector<std::string> vals = split(line, " ");
		std::unordered_set<int> PVS;
		for (std::string idx_s : vals) {
			PVS.emplace(std::stoi(idx_s));
		}
		PVSs[i] = PVS;
		i++;
	}
	return true;
}

bool Scene::parse(string filename) {
  std::ifstream file(filename, ios::in);
	if (!file.is_open()) {
		cout << "Map file not found" << endl;
		return false;
	}
  std::string line; 
  unsigned int size;
  unsigned int i = 0;
  while (std::getline(file, line)) {
    size = line.size(); 
    map->addRow(size);
    for (unsigned int j= 0; j< size; j++) {
      switch (line[j]) {
        case 'w':
          map->addWall(i,j);
          break;
        case ' ':
          map->addFloor(i,j);
          break;
				case 'b':
					map->addBunny(i,j);
					break;
				case 'a':
					map->addArmadillo(i,j);
					break;
				case 'm':
					map->addMax(i,j);
					break;
				case 'o':
					map->addMoai(i,j);
					break;
				case 'h':
					map->addHorse(i,j);
					break;
				case 'l':
					map->addLucy(i,j);
					break;
				case 'f':
					map->addFrog(i,j);
					break;
				case 'd':
					map->addDragon(i,j);
					break;
				case 'p':
					map->addPlayer(i,j);
        default:
          break;
      }
    }
    i++;
  }
	file.close();
	return true;
}

bool Scene::init(int argc, char* argv[])
{
	initShaders();

	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	if (args.size() != 3) {
		cout << "Usage: ./BaseCode file.map file.vis" << endl;
		cout << "Now parsing ../maps/3.map and ../visibility/results/3.vis" << endl;
		if (!parse( "../maps/3.map")) {
			return false;
		}
		if (!parseVisibility("../visibility/results/3.vis")) {
			return false;
		}
	} else {
		if (!parse(args[1])) return false;
		if (!parseVisibility(args[2])) return false;
	}
	// Parse the map
	// Parse the visibility

	vector<char> possibilities = {'a', 'b', 'd', 'f', 'h', 'l',
																'w', 'm', 'o', 'r'};
	std::map<char, bool> is_comp;
	for (char c : possibilities) is_comp[c] = false;

	bool is_mesh;
	std::string mesh_location;
	
	// Build mesh instances
	for (char c : map->getTypesOfMesh()) {
		TriangleMesh* mesh = new TriangleMesh();
		bool success;
		LOD lod;
		PLYReader reader;
		if (is_comp[c]) {
			free(mesh);
			continue;
		}
		is_mesh = false;
		switch (c) {
			case 'w':
				mesh->buildCube();
				mesh->sendToOpenGL(basicProgram, is_mesh);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				is_comp['w'] = true;
				break;
			case ' ':
				mesh->buildFloor();
				mesh->sendToOpenGL(basicProgram, is_mesh);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				is_comp[' '] = true;
				break;
			case 'b':
				mesh_location = "../models/bunny.ply";
				is_mesh = true;
				break;
			case 'a':
				mesh_location = "../models/Armadillo.ply";
				is_mesh = true;
				break;
			case 'l':
				mesh_location = "../models/lucy.ply";
				is_mesh = true;
				break;
			case 'f':
				mesh_location = "../models/frog.ply";
				is_mesh = true;
				break;
			case 'h':
				mesh_location = "../models/horse.ply";
				is_mesh = true;
				break;
			case 'd':
				mesh_location = "../models/dragon.ply";
				is_mesh = true;
				break;
			case 'm':
				mesh_location = "../models/maxplanck.ply";
				is_mesh = true;
				break;
			case 'o':
				mesh_location = "../models/moai.ply";
				is_mesh = true;
				break;	
			default:
				break;
		}
		if (is_mesh) {
			cout << "Reading " << mesh_location << " file." << endl;
			success = reader.readMesh(mesh_location, *mesh);
			if (!success) continue;
			lod = LOD(mesh);
			assert(lod.simp_vertices.size() != 0);
			mesh->setLOD(lod);
			mesh->setLODlevel(0);
			mesh->sendToOpenGL(basicProgram, is_mesh);
			meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
			is_comp[c] = true;
		}
	}
	currentTime = 0.0f;
	
	camera.init(2.0f);
	// Set player position
	camera.setPlayer(-map->player_i, -map->player_j);
	
	bPolygonFill = true;
	vis_mode = false;
	
	// Select which font you want to use
	if(!text.init("fonts/OpenSans-Regular.ttf"))
	//if(!text.init("fonts/OpenSans-Bold.ttf"))
	//if(!text.init("fonts/DroidSerif.ttf"))
		cout << "Could not load font!!!" << endl;
	return true;
}

bool Scene::loadMesh(const char *filename)
{
	return false;
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay) 
{
	if (!bPlay) return;
	currentTime += deltaTime;
	framerate = 1.0f/deltaTime * 1000;
	
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	glm::vec3 mov(0);
	// Get forward vector of camera
	glm::vec3 forw;
	if (!vis_mode) forw = glm::normalize(glm::vec3(m[0][2], 0, m[2][2]));
	else forw = glm::normalize(glm::vec3(m[0][2], m[1][2], m[2][2]));
	glm::vec3 side = glm::normalize(glm::cross(forw, glm::vec3(0,1,0)));
	float rate = float(1.0f/deltaTime);
	if (forward){
		mov += rate * forw;
	} 
	if (back){
		mov -= rate * forw;
	}
	if (left){
		mov -= rate * side;
	} 
	if (right){
		mov += rate * side;
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
	glm::vec3 player_pos = camera.getPosition();
	int player_i = -floor(player_pos[0]);
	int player_j = -floor(player_pos[2]);
	basicProgram.setUniform1i("bLighting", bPolygonFill?1:0);
	int size_col = map->layout[0].size();
	int s_row = map->layout.size();
	tcr.getBestLODs(map, meshes, framerate, delta, PVSs[player_i*size_col + player_j], player_pos);

	basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	// Iterate over the visible map cells from the player POV
	if (!vis_mode) {
		for (int i : PVSs[player_i*size_col + player_j]) {
			int row = i / size_col;
			int col = i % size_col;
			
			// Change color depending on the object
			if (map->layout[row][col] == ' ') basicProgram.setUniform4f("color", 0.40, 0.27, 0.00, 1);
			else if (map->layout[row][col] == 'w') basicProgram.setUniform4f("color", 0.68, 0.68, 0.68, 1);
			else basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
			
			// Translate instance to position
			glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(row, 0, col));
			glm::mat4 mat = MV * matrix;
			basicProgram.setUniformMatrix4f("modelview", mat);
			
			TriangleMesh* m = meshes[map->layout[row][col]];

			// Render a mesh from a different color depending on its LOD
			if (map->layout[row][col] != ' ' && map->layout[row][col] != 'w') {
				m->setLODlevel(map->lodLevel[row*size_col+col]);
				if (m->LOD_level == 0) basicProgram.setUniform4f("color", 1, 0, 0, 1.0);
				else if (m->LOD_level == 1) basicProgram.setUniform4f("color", 0, 1, 0, 1.0);
				else if (m->LOD_level == 2) basicProgram.setUniform4f("color", 0, 0, 1, 1.0);
				else basicProgram.setUniform4f("color", 0.9, 0.9, 0.9, 1.0);
			}
			m->render();
			// Draw a floor if the object was a mesh
			if (map->layout[row][col] != ' ' && map->layout[row][col] != 'w') {
					TriangleMesh * floor = meshes[' '];
					basicProgram.setUniform4f("color", 0, 0.27, 0.4, 1.0);
					floor->render();
			}
		}
	} else {
		for (int i = 0; i<s_row; i++) {
			int s = map->layout[i].size();
			for (int j = 0; j<s; j++) {
				TriangleMesh* m = meshes[map->layout[i][j]];
				if (PVSs[player_i*s + player_j].find(i*s + j) == PVSs[player_i*s + player_j].end()) {
					basicProgram.setUniform4f("color", 0, 1, 0, 1);
				} else basicProgram.setUniform4f("color", 1, 0, 0, 1);

				if(m) {
					m->setLODlevel(0);
					glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
					glm::mat4 mat = MV * matrix;
					basicProgram.setUniformMatrix4f("modelview", mat);
					m->render();
				}
				if (map->layout[i][j] != ' ' && map->layout[i][j] != 'w') {
						TriangleMesh * floor = meshes[' '];
						floor->render();
				}
			}
		}
	}

	text.render(std::to_string(framerate), glm::vec2(20, 20), 16, glm::vec4(0, 0, 0, 1));
	
	//cout << "Time render map: " << (end-begin) / CLOCKS_PER_SEC << endl;
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
	for (auto el : meshes) 
		free(el.second);
}

void Scene::setDelta(double del) {
	delta = del;
}

void Scene::toggleVisMode() {
	vis_mode = !vis_mode;
}