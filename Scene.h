#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	bool loadMesh(const char *filename);
	void update(int deltaTime);
	void render();

  Camera &getCamera();
  
  void switchPolygonMode();

private:
	void initShaders();
	void computeModelViewMatrix();

private:
  Camera camera;
	TriangleMesh *mesh;
	ShaderProgram basicProgram;
	float currentTime;
	
	bool bPolygonFill;

};


#endif // _SCENE_INCLUDE
