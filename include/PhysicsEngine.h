#pragma once

#include <btBulletDynamicsCommon.h>
#include "utils.h"


class PhysicsEngine
{

public:
	PhysicsEngine();
	void initialise();
	void close();
	btTransform bullet_step_sphere(int i, float time);
	btRigidBody* SetSphere(float size, btTransform T, btVector3 velocity);
	std::vector<btRigidBody*> getSphereMovingBits();

};

