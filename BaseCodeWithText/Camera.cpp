#include "Camera.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#define PI 3.14159f


Camera::Camera()
{
}

Camera::~Camera()
{
}


void Camera::init(float initDistance, float initAngleX, float initAngleY)
{
  position = glm::vec3(0, -1, 0);
  distance = initDistance;
  angleX = initAngleX;
  angleY = initAngleY;
 	rangeDistanceCamera[0] = 1.0f;
	rangeDistanceCamera[1] = 3.0f;
	computeModelViewMatrix();
}
	
void Camera::resizeCameraViewport(int width, int height)
{
  projection = glm::perspective(60.f / 180.f * PI, float(width) / float(height), 0.01f, 100.0f);
}

void Camera::rotateCamera(float xRotation, float yRotation)
{
  angleX += xRotation;
  angleX = glm::max(-75.0f, glm::min(angleX, 75.0f));
  angleY += yRotation;
	computeModelViewMatrix();
}

void Camera::zoomCamera(float distDelta)
{
  distance += distDelta;
  distance = glm::max(rangeDistanceCamera[0], glm::min(distance, rangeDistanceCamera[1]));
	computeModelViewMatrix();
}

void Camera::computeModelViewMatrix()
{
	modelview = glm::mat4(1.0f);
	// modelview = glm::rotate(modelview, angleX / 180.f * PI, glm::vec3(modelview[0]));
	modelview = glm::rotate(modelview, angleX / 180.f * PI, glm::vec3(1.0f, 0.0f, 0.0f));
	// modelview = glm::rotate(modelview, angleY / 180.f * PI, glm::vec3(modelview[1]));
	modelview = glm::rotate(modelview, angleY / 180.f * PI, glm::vec3(0.0f, 1.0f, 0.0f));
	// modelview = glm::translate(modelview, glm::vec3(0.0f, 0.0f, -distance));
  modelview = glm::translate(modelview, position);
}

glm::mat4 &Camera::getProjectionMatrix()
{
  return projection;
}

glm::mat4 &Camera::getModelViewMatrix()
{
  return modelview;
}

void Camera::move(glm::vec3 mov) {
  position += mov;
  computeModelViewMatrix();
}

glm::vec3 &Camera::getPosition() {
  return position;
}

void Camera::setPlayer(int i, int j) {
  position = glm::vec3(i, -1, j);
}