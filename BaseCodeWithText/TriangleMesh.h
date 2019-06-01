#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "LOD.h"
using namespace std;

// Class TriangleMesh renders a very simple room with textures

class TriangleMesh
{

public:
	TriangleMesh();

	void addVertex(const glm::vec3 &position);
	void addTriangle(int v0, int v1, int v2);

	void buildCube();
	void buildFloor();
	vector<glm::vec3> getVertices();
	vector<int> getTriangles();
	void setLOD(LOD);
	void setLODlevel(int);
	void sendToOpenGL(ShaderProgram &program);
	void render() const;
	void free();

private:
  vector<glm::vec3> vertices;
  vector<int> triangles;
	LOD lod;
	int LOD_level;
	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;
	
};


#endif // _TRIANGLE_MESH_INCLUDE


