#include "Camera.h"
#include <utils.h>
#include <glm\gtx\transform.hpp>
#include <glm\gtx\spline.hpp>

const float Camera::MOVEMENT_SPEED = 0.2f;
const float Camera::ROTATIONAL_SPEED = 0.2f;
const float Camera::MIN_Y = 1.0f;
const float Camera::lookSpeed = 0.1f;
const float Camera::sphereLim = 20000;

Camera::Camera() :
	viewDirection(-0.7169f, 0.0450f, 0.6531f),
	position(107.1882f, 24.8962f, -38.6533f),
	UP(0.0f, 1.0f, 0.0f)
{
		
}

void Camera::mouseUpdate(const glm::vec2& newMousePosition) 
{
	glm::vec2 mouseDelta = newMousePosition - oldMousePosition;

	// stop jitters when going off screen
	if (glm::length(mouseDelta) > 1.0f)
	{
		oldMousePosition = newMousePosition;
		return;
	}
	strafeDirection = glm::cross(viewDirection, UP);

	glm::mat4 rotator = glm::rotate(-mouseDelta.x * ROTATIONAL_SPEED, UP) *
		glm::rotate(-mouseDelta.y * ROTATIONAL_SPEED, strafeDirection);

	viewDirection = glm::mat3(rotator) * viewDirection;

	oldMousePosition = newMousePosition;	
}


glm::mat4 Camera::getWorldToViewMatrix() const 
{
	return glm::lookAt(position, position + viewDirection, UP);
}

void Camera::loadHeightData(uint dimensions, Texture terrain) {

	int numVertices = dimensions * dimensions;
	int half = dimensions / 2;

	for (int row = 0; row < dimensions; row++)
	{
		for (int col = 0; col < dimensions; col++)
		{
			
			unsigned char color = (unsigned char)terrain.data[3 * (col * terrain.width + row)];
			float h = terrain.height * ((color / 255.0f) - 0.5f);
			//scaling
			h = h / 5.0f;
			heightMap[row][col] = h;

		}
	}
}

void Camera::lookUp()
{
	strafeDirection = glm::cross(viewDirection, UP);

	glm::mat4 rotator = glm::rotate(lookSpeed * ROTATIONAL_SPEED, strafeDirection);

	viewDirection = glm::mat3(rotator) * viewDirection;
}

void Camera::lookDown()
{
	strafeDirection = glm::cross(viewDirection, UP);

	glm::mat4 rotator = glm::rotate(-lookSpeed * ROTATIONAL_SPEED, strafeDirection);

	viewDirection = glm::mat3(rotator) * viewDirection;
}

void Camera::lookLeft()
{
	strafeDirection = glm::cross(viewDirection, UP);

	glm::mat4 rotator = glm::rotate(lookSpeed * ROTATIONAL_SPEED, UP);

	viewDirection = glm::mat3(rotator) * viewDirection;
}

void Camera::lookRight()
{
	strafeDirection = glm::cross(viewDirection, UP);

	glm::mat4 rotator = glm::rotate(-lookSpeed * ROTATIONAL_SPEED, UP);

	viewDirection = glm::mat3(rotator) * viewDirection;
}

void Camera::moveForwards()
{
	glm::vec3 newPosition = position + (MOVEMENT_SPEED * viewDirection);

	if ( ((newPosition.x * newPosition.x) + (newPosition.y * newPosition.y) + (newPosition.z * newPosition.z) < sphereLim) &
		(heightMap[(int) (150 + newPosition.z)][(int)(150 + newPosition.x)] + 2 < newPosition.y))
		position = newPosition;


	if (position.y < MIN_Y)
		position.y = MIN_Y;
}

void Camera::moveBackwards()
{
	glm::vec3 newPosition = position + (-MOVEMENT_SPEED * viewDirection);

	if (((newPosition.x * newPosition.x) + (newPosition.y * newPosition.y) + (newPosition.z * newPosition.z) < sphereLim) &
		(heightMap[(int)(150 + newPosition.z)][(int)(150 + newPosition.x)] + 2 < newPosition.y))
		position = newPosition;

	if (position.y < MIN_Y)
		position.y = MIN_Y;
}

void Camera::strafeLeft()
{
	position += -MOVEMENT_SPEED * strafeDirection;
}

void Camera::strafeRight()
{
	position += MOVEMENT_SPEED * strafeDirection;
}

void Camera::moveUp()
{
	glm::vec3 newPosition =  position + (MOVEMENT_SPEED * UP);

	if (((newPosition.x * newPosition.x) + (newPosition.y * newPosition.y) + (newPosition.z * newPosition.z) < sphereLim) &
		(heightMap[(int)(150 + newPosition.z)][(int)(150 + newPosition.x)] + 2 < newPosition.y))
		position = newPosition;

}

void Camera::moveDown()
{
	if (position.y > MIN_Y) {

		glm::vec3 newPosition = position - (MOVEMENT_SPEED * UP);

		if (((newPosition.x * newPosition.x) + (newPosition.y * newPosition.y) + (newPosition.z * newPosition.z) < sphereLim) &
			(heightMap[(int)(150 + newPosition.z)][(int)(150 + newPosition.x)] + 2 < newPosition.y))
			position = newPosition;

	}
	
}

void Camera::goToScreenShotPos() 
{
	// if the user was flying themselves around, save the coords 
	if (!cameraIsInPredefinedViewPos) {
		lastPosition = position;
		lastViewDirection = viewDirection;
		cameraIsInPredefinedViewPos = true;
	}

	viewDirection = glm::vec3(-0.7087f, -0.4938f, 0.4432f);
	position = glm::vec3(96.6133f, 85.0036f, -70.7279f);

}

void Camera::goToNextPos()
{

	// if the user was flying themselves around, save the coords 
	if (!cameraIsInPredefinedViewPos) {
		lastPosition = position;
		lastViewDirection = viewDirection;
		cameraIsInPredefinedViewPos = true;
	}

	viewIndex = viewIndex + 1;

	glm::vec3 viewDirections[] = {
		glm::vec3(-0.4885f, 0.1224f, 0.8300f),
		glm::vec3(-0.7169f, 0.0450f, 0.6531f),	
		glm::vec3(0.2274f, -0.9389f, -0.0963f),
		glm::vec3(-0.2428f, 0.0062f, -0.9400f)
	};


	glm::vec3 viewPostions[] = {
		glm::vec3(-8.8714f, 8.3117f, -75.1934f),
		glm::vec3(107.1882f, 24.8962f, -38.6533f),
		glm::vec3(-30.6159f, 129.6599f, 18.3606f),
		glm::vec3(6.8396f, 0.8061f, 106.7234f)
	};

	if (viewIndex == (sizeof(viewDirections) / sizeof(viewDirections[0])))
		viewIndex = 0;

	viewDirection = viewDirections[viewIndex];
	position = viewPostions[viewIndex];
	print(position);

}

void Camera::returnToUserControl()
{
	if (cameraIsInPredefinedViewPos) {
		position = lastPosition;
		viewDirection = lastViewDirection;
		cameraIsInPredefinedViewPos = false;
	}
}

void Camera::printDetails() 
{
	print(viewDirection, position);
}

int Camera::goToNextTourPos(float t) {

	float closeness = 0.2f;
		

	glm::vec3 cp[] = {
		glm::vec3(-11.6408,  46.2874, 126.2104),
		glm::vec3(-11.6408,  46.2874, 126.2104),
		glm::vec3(-82.6326,  37.0386, 100.0372),
		glm::vec3(-103.4403,  32.3185,  44.1307),
		glm::vec3(-69.8106,  20.7779,  10.5602),

		glm::vec3(-45.8478, 12.4150, -41.3281),
		glm::vec3(-62.1435, 11.5437, -71.8562),
		//glm::vec3(-19.4390, 10.9241, -111.6210),

		//glm::vec3(-42.9814,  11.6764, -49.1075),
		glm::vec3(-25.3662,  10.5677, -111.7441),
		glm::vec3(6.4208,  10.5677, -104.6214),
		glm::vec3(20.3958,  14.4321,  -7.7725),
		glm::vec3(-0.3209,  28.1797,  85.7421),
		glm::vec3(51.5313,  28.1797,  90.2816),
		glm::vec3(88.3756,  47.1798,  31.0643),
		glm::vec3(32.0218,  38.7287,  12.2199),
		glm::vec3(-15.1158,  32.1372,  61.1684),
		glm::vec3(-62.4476,  29.4497,  43.2418),
		glm::vec3(-42.5365,  93.2975, -73.5391),
		glm::vec3(8.6349, 101.5620, -102.7257),
		glm::vec3(68.0970, 101.5620, -72.7608),
		glm::vec3(88.3354, 101.5620, -37.7505),
		glm::vec3(94.0416, 103.7723,  -5.0158),
		glm::vec3(86.1674, 103.7723,  20.6544),
		glm::vec3(79.7398, 107.9327,  44.6221),
		glm::vec3(49.2089, 110.5329,  72.9101),
		glm::vec3(8.3507, 110.6629,  87.5854),
		glm::vec3(-30.8758, 110.9229,  86.8089),
	};

	glm::vec3 vp[] = {
		glm::vec3(-0.7493,  -0.1101,  -0.6074),
		glm::vec3(-0.7493,  -0.1101,  -0.6074),
		glm::vec3(-0.0287,  -0.0908,  -0.9662),
		glm::vec3(0.2722,  -0.0908,  -0.8434),
		glm::vec3(0.6642,  -0.2623,  -0.6577),

		glm::vec3(-0.1092, -0.1305, -1.0155),
		glm::vec3(1.0221, -0.1101, -0.0592),
		//glm::vec3(-0.0935, -0.0896, 1.0215),

		//glm::vec3(-0.1629,  -0.1101,  -0.9507),
		glm::vec3(0.2224,   0.0256,   0.9447),
		glm::vec3(-0.4727,   0.0256,   0.8476),
		glm::vec3(0.2601,  -0.0132,   0.9353),
		glm::vec3(0.6562,  -0.2995,  -0.6498),
		glm::vec3(-0.5739,  -0.2058,  -0.7555),
		glm::vec3(-0.8964,  -0.3724,   0.0169),
		glm::vec3(-0.7693,  -0.2248,   0.5479),
		glm::vec3(-0.6182,  -0.2058,  -0.7197),
		glm::vec3(0.3749,  -0.2436,  -0.8618),
		glm::vec3(0.5374,  -0.5431,   0.5990),
		glm::vec3(0.1272,  -0.6207,   0.7356),
		glm::vec3(-0.5411,  -0.6500,   0.4766),
		glm::vec3(-0.6691,  -0.6500,   0.2688),
		glm::vec3(-0.7176,  -0.6500,  -0.0705),
		glm::vec3(-0.6590,  -0.6500,  -0.2927),
		glm::vec3(-0.5793,  -0.6500,  -0.4294),
		glm::vec3(-0.3787,  -0.6500,  -0.6137),
		glm::vec3(0.0340,  -0.6500,  -0.7203),
		glm::vec3(0.0340,  -0.6500,  -0.7203),
	};



	position = glm::catmullRom(cp[tourIndex], cp[tourIndex+1], cp[tourIndex+2], cp[tourIndex+3], t);

	viewDirection = glm::catmullRom(vp[tourIndex], vp[tourIndex + 1], vp[tourIndex + 2], vp[tourIndex + 3], t);

	if ((abs(position.x - cp[tourIndex + 2].x) < closeness) &&
		(abs(position.y - cp[tourIndex + 2].y) < closeness) &&
		(abs(position.z - cp[tourIndex + 2].z) < closeness) &&
		(abs(viewDirection.x - vp[tourIndex + 2].x) < closeness) &&
		(abs(viewDirection.y - vp[tourIndex + 2].y) < closeness) &&
		(abs(viewDirection.z - vp[tourIndex + 2].z) < closeness))
	{
		tourIndex++;
		// is tour finished?
		if (tourIndex + 3 == (sizeof(vp) / sizeof(vp[0]))) {
			return 4;
		}
		else 
		{
			// just say we need to reset the counter so we can start interpolating to next pos
			return 3;
		}
	}
	return 1;

}
