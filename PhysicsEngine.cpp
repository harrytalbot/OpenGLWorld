#include "physicsEngine.h"
#include <math.h>
#include <time.h>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()

#include <iostream>
#include <string.h>
#include <stddef.h> // must include for the offsetof macro 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Primitives\ShapeGenerator.h>
#include <Primitives\Vertex.h>

#include "utils.h"
#include "World.h"
/*
* Set up bullet - globals.
*/
#include <btBulletDynamicsCommon.h>

btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

std::vector<btRigidBody*> SphereMovingBits; // so that can get at all bits
float fireballYDeleteHeight = 0.0f;
float fireballYStart = -10.0f;
glm::vec3 fireballLocations[] = {
	glm::vec3(11.0f, fireballYStart, 19.0f),
	glm::vec3(17.5f, fireballYStart, -5.0f),
	glm::vec3(17.7f, fireballYStart, 14.5f),
	glm::vec3(34.0f, fireballYStart, 15.2f),
	glm::vec3(-9.0f, fireballYStart, 65.0f),
	glm::vec3(-4.0f, fireballYStart, 52.0f),
	glm::vec3(14.0f, fireballYStart, 57.5f),
	glm::vec3(9.2f, fireballYStart, 69.0f),
	glm::vec3(7.9f, fireballYStart, 77.5f),
	glm::vec3(18.6f, fireballYStart, 89.0f),

	glm::vec3(-56.5f, fireballYStart, -12.7f),
	glm::vec3(-73.9f, fireballYStart, -37.0f),
	glm::vec3(-64.9f, fireballYStart, -61.5f),
	glm::vec3(-39.3f, fireballYStart, -54.0f),
	glm::vec3(-34.2f, fireballYStart, -33.0f),
	glm::vec3(-6.7f, fireballYStart, -55.0f),
	glm::vec3(3.3f, fireballYStart, -78.0f),
	glm::vec3(-6.0f, fireballYStart, -89.0f),
	glm::vec3(-31.0f, fireballYStart, -86.4f),
};

PhysicsEngine::PhysicsEngine()
{

}

void PhysicsEngine::initialise() 
{
	print("Initialising Bullet...");
	// set up world	
	broadphase = new btDbvtBroadphase();
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0., GRAVITY, 0));

	// Set up lava floor to bounce off
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -30, 0)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, -0, 0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	groundRigidBody->setRestitution(COE);
	dynamicsWorld->addRigidBody(groundRigidBody);


	srand(time(0));  // Initialize random number generator.
	float r = (rand() % 10) + 1;


	// add all the fireballs

	for (int i = 0; i < (sizeof(fireballLocations) / sizeof(fireballLocations[0])); i++)
	{
		SphereMovingBits.push_back(SetSphere(1., btTransform(btQuaternion(0, 0, 0, 1), 
			btVector3(fireballLocations[i].x, ((rand()% 40)-40), fireballLocations[i].z)), btVector3(0.0f, 5.0f, 0.0f)));
	}

	print("...Done.");
	print("Moving Spheres: " + SphereMovingBits.size());
}

btTransform PhysicsEngine::bullet_step_sphere(int i, float time) {
	btTransform trans;
	btRigidBody* moveRigidBody;
	int n = SphereMovingBits.size();
	moveRigidBody = SphereMovingBits[i];
	dynamicsWorld->stepSimulation(1 / 800.f, 10);
	//dynamicsWorld->stepSimulation(time / 1000, 1);
	moveRigidBody->getMotionState()->getWorldTransform(trans);
	return trans;
}

btRigidBody* PhysicsEngine::SetSphere(float size, btTransform T, btVector3 velocity) {
	btCollisionShape* shape = new btSphereShape(size);
	btDefaultMotionState* MotionState = new btDefaultMotionState(T);
	btScalar mass = 1;
	btVector3 Inertia(0.1f, 0.1f, 0.5f);
	shape->calculateLocalInertia(mass, Inertia);
	btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, MotionState, shape, Inertia);
	btRigidBody* RigidBody = new btRigidBody(RigidBodyCI);
	RigidBody->setLinearVelocity(velocity);
	RigidBody->setRestitution(COE);
	RigidBody->setFriction(0.0f);
	RigidBody->setRollingFriction(0.0f);
	RigidBody->setDamping(0.0f, 0.0f);
	dynamicsWorld->addRigidBody(RigidBody);
	return RigidBody;
}

std::vector<btRigidBody*> PhysicsEngine::getSphereMovingBits() {
	return SphereMovingBits;
}


void PhysicsEngine::close() {
	/*
	* This is very minimal and relies on OS to tidy up.
	*/
	btRigidBody* moveRigidBody;
	moveRigidBody = SphereMovingBits[0];
	dynamicsWorld->removeRigidBody(moveRigidBody);
	delete moveRigidBody->getMotionState();
	delete moveRigidBody;
	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;
}