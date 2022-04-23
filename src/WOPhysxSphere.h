#pragma once

#include "WO.h"
#include "PxPhysicsAPI.h"
#include "vehicle/PxVehicleSDK.h"

namespace Aftr
{

class WOPhysxSphere : public WO 
{
public:
	static WOPhysxSphere* New(const std::string & pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, const Vector& initialPos );
	virtual ~WOPhysxSphere();
	virtual void onUpdateWO() override;
	void setDirection(Vector dir, float power);
	void setActorPose(const Mat4& pose);

	physx::PxRigidDynamic* actor;

protected:
	WOPhysxSphere();
	virtual void onCreate(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, const Vector& initialPos );
	Aftr::Mat4 convertMatrix(const physx::PxTransform& currentTransform);
	physx::PxTransform convertTransform(const Mat4& mat);

	
	physx::PxPhysics* p;
	physx::PxScene* scene;
	Vector initialPos;
	Vector forceDirection;
	bool setForce;
};


} //namespace Aftr
