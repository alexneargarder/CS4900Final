#include "WOPhysxBox.h"

#include "Mat4.h"
#include "Model.h"

using namespace Aftr;

WOPhysxBox* WOPhysxBox::New(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, Camera* cam, physx::PxRigidDynamic* camera)
{
	WOPhysxBox* wo = new WOPhysxBox();
	wo->onCreate(pathToModel, scale, p, scene, cam, camera);
	return wo;
}

WOPhysxBox::WOPhysxBox() : WO(), IFace(this), actor(nullptr)
{
}

WOPhysxBox::~WOPhysxBox()
{

}

void WOPhysxBox::onCreate(const std::string& pathToModel, const Vector& scale, physx::PxPhysics* p, physx::PxScene* scene, Camera* cam, physx::PxRigidDynamic* camera)
{
	WO::onCreate( pathToModel, scale );
	//this->getModel()->getBoundingBox().getlxlylz()

	this->p = p;
	this->scene = scene;
	this->cam = cam;
	this->camera = camera;


	this->upon_async_model_loaded([this]()
		{
			this->initialPos = this->cam->getPosition() + this->cam->getLookDirection() * 5;
			this->initialPose = this->cam->getPose();
			this->force = this->cam->getLookDirection() * 100 + Vector( this->camera->getLinearVelocity().x, this->camera->getLinearVelocity().y, this->camera->getLinearVelocity().z);

			physx::PxMaterial* gMaterial = this->p->createMaterial(0.5f, 0.5f, 0.6f);

			physx::PxShape* shape = this->p->createShape(physx::PxBoxGeometry(this->getModel()->getBoundingBox().getlxlylz().x / 2, this->getModel()->getBoundingBox().getlxlylz().y / 2, this->getModel()->getBoundingBox().getlxlylz().z / 2), *gMaterial, true);

			physx::PxTransform t({ this->initialPos.x, this->initialPos.y, this->initialPos.z });

			actor = this->p->createRigidDynamic(t);
			if (actor == nullptr)
				std::cout << "initiallity null" << std::endl;
			else
				std::cout << "intiiallty nonnull " << std::endl;

			if (!actor->attachShape(*shape))
				std::cout << "attaching shape failed" << std::endl;

			actor->userData = this;

			this->scene->addActor(*actor);

			this->setPose(this->initialPose);
			this->rotateAboutRelZ(PI / 2);
			this->setPosition(this->initialPos);
			this->setActorPose(this->getPose());

			this->actor->addForce( physx::PxVec3( this->force.x, this->force.y, this->force.z ), physx::PxForceMode::eVELOCITY_CHANGE);
		});
	
}

Aftr::Mat4 WOPhysxBox::convertMatrix(const physx::PxTransform& currentTransform)
{
	physx::PxMat44 pxmat(currentTransform);
	Aftr::Mat4 aftrmat;

	for (int i = 0; i < 16; ++i)
	{
		aftrmat[i] = pxmat(i % 4, i / 4);
	}

	return aftrmat;
}

physx::PxTransform WOPhysxBox::convertTransform(const Mat4& mat)
{
	physx::PxMat44 pxmat;


	for (int i = 0; i < 16; ++i)
	{
		pxmat(i % 4, i / 4) = mat[i];
	}

	physx::PxTransform ptransform( pxmat );

	return ptransform;
}

void WOPhysxBox::onUpdateWO()
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

void WOPhysxBox::setActorPose(const Mat4& pose)
{
	actor->setGlobalPose(convertTransform(pose));
}


