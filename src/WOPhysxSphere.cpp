#include "WOPhysxSphere.h"

#include "Mat4.h"
#include "Model.h"
#include "BoundingBox.h"

using namespace Aftr;

WOPhysxSphere* WOPhysxSphere::New(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, const Vector& initialPos )
{
	WOPhysxSphere* wo = new WOPhysxSphere();
	wo->onCreate(pathToModel, scale, p, scene, initialPos );
	return wo;
}

WOPhysxSphere::WOPhysxSphere() : WO(), IFace(this), actor(nullptr)
{
	actor = nullptr;
	forceDirection = Vector({ 0, 0, 0 });
	setForce = false;
}

WOPhysxSphere::~WOPhysxSphere()
{

}

void WOPhysxSphere::onCreate(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, const Vector& initialPos)
{
	WO::onCreate( pathToModel, scale );

	this->p = p;
	this->scene = scene;
	this->initialPos = initialPos;

	this->upon_async_model_loaded([this]()
		{
			physx::PxMaterial* gMaterial = this->p->createMaterial(0.5f, 0.5f, 0.6f);
			physx::PxShape* shape = this->p->createShape(physx::PxSphereGeometry( this->getModel()->getBoundingBox().getlxlylz().x / 2 ), *gMaterial, true);

			physx::PxTransform t({ this->initialPos.x, this->initialPos.y, this->initialPos.z });

			this->actor = this->p->createRigidDynamic(t);
			if (actor == nullptr)
				std::cout << "initiallity null" << std::endl;
			else
				std::cout << "intiiallty nonnull " << std::endl;

			if (!actor->attachShape(*shape))
				std::cout << "attaching shape failed" << std::endl;


			actor->setContactReportThreshold(1);
			actor->userData = this;

			this->scene->addActor(*actor);

			if ( setForce )
				actor->addForce(physx::PxVec3(forceDirection.x, forceDirection.y, forceDirection.z), physx::PxForceMode::eVELOCITY_CHANGE);
		});

	
}

Aftr::Mat4 WOPhysxSphere::convertMatrix(const physx::PxTransform& currentTransform)
{
	physx::PxMat44 pxmat(currentTransform);
	Aftr::Mat4 aftrmat;

	for (int i = 0; i < 16; ++i)
	{
		aftrmat[i] = pxmat(i % 4, i / 4);
	}

	return aftrmat;
}

void WOPhysxSphere::onUpdateWO()
{
	if (actor != nullptr)
	{
		//std::cout << "updating pose" << std::endl;
		setPose(convertMatrix(actor->getGlobalPose()));
	}
	else
	{
		//std::cout << "null" << std::endl;
	}
}

void WOPhysxSphere::setDirection( Vector dir, float power )
{
	forceDirection = dir * power;

	if (actor != nullptr)
		actor->addForce(physx::PxVec3(forceDirection.x, forceDirection.y, forceDirection.z), physx::PxForceMode::eVELOCITY_CHANGE);
	else
		setForce = true;
		
}

physx::PxTransform WOPhysxSphere::convertTransform(const Mat4& mat)
{
	physx::PxMat44 pxmat;


	for (int i = 0; i < 16; ++i)
	{
		pxmat(i % 4, i / 4) = mat[i];
	}

	physx::PxTransform ptransform(pxmat);

	return ptransform;
}

void WOPhysxSphere::setActorPose(const Mat4& pose)
{
	actor->setGlobalPose(convertTransform(pose));
}





