#pragma once
#include <glm\glm.hpp>
#include <Primitives\ShapeData.h>
#include "Texture.h"

typedef unsigned int uint;

class Camera
{
	glm::vec3 position;
	glm::vec3 viewDirection;

	float heightMap[300][300];

	bool cameraIsInPredefinedViewPos = false;
	glm::vec3 lastPosition;
	glm::vec3 lastViewDirection;

	const glm::vec3 UP;
	glm::vec2 oldMousePosition;
	static const float MOVEMENT_SPEED;
	static const float ROTATIONAL_SPEED;
	static const float MIN_Y;
	static const float lookSpeed;
	static const float sphereLim;

	glm::vec3 strafeDirection;

	int viewIndex = 0;
	int tourIndex = 0;

public:
	Camera();
	void mouseUpdate(const glm::vec2& newMousePosition);
	glm::mat4 getWorldToViewMatrix() const;

	void loadHeightData(uint dimensions, Texture terrain);

	void lookUp();
	void lookDown();
	void lookLeft();
	void lookRight();

	void moveForwards();
	void moveBackwards();
	void strafeLeft();
	void strafeRight();
	void moveUp();
	void moveDown();

	void printDetails();
	void goToScreenShotPos();
	void goToNextPos();
	void returnToUserControl();

	int goToNextTourPos(float t);


};

