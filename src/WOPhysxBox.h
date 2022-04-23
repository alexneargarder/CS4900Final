#pragma once

#include "WO.h"
#include "PxPhysicsAPI.h"
#include "vehicle/PxVehicleSDK.h"
#include "Mat4.h"
#include "Camera.h"

namespace Aftr
{

class WOPhysxBox : public WO 
{
public:
	static WOPhysxBox* New(const std::string & pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, Camera* cam, physx::PxRigidDynamic* camera);
	virtual ~WOPhysxBox();
	virtual void onUpdateWO() override;
	void setActorPose(const Mat4& pose);

	physx::PxRigidDynamic* actor;

protected:
	WOPhysxBox();
	virtual void onCreate(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, Camera* cam, physx::PxRigidDynamic* camera);
	Aftr::Mat4 convertMatrix(const physx::PxTransform& currentTransform);
	physx::PxTransform convertTransform(const Mat4& mat);

	physx::PxPhysics* p;
	physx::PxScene* scene;
	Vector initialPos;
	Vector force;
	Mat4 initialPose;
	Camera* cam;
	physx::PxRigidDynamic* camera;
};


} //namespace Aftr
