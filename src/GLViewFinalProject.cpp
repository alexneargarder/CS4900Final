#include "GLViewFinalProject.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "SDL_gamecontroller.h"
#include <ctime>
#include <ratio>
#include <chrono>
#include "WOGUILabel.h"
#include "irrKlang.h"

#ifdef VR
#include "SDL_syswm.h"
#include "AftrOpenGLIncludes.h"
#include "VRRenderer.h"
#endif

#include "WOPhysxSphere.h"


bool rb = false;
bool lb = false;
bool dup = false;
bool ddown = false;
bool dleft = false;
bool dright = false;
bool rollLeft = false;
bool rollRight = false;

using namespace Aftr;
bool course2 = false;
bool start = true;
std::chrono::steady_clock::time_point t1;
std::chrono::steady_clock::time_point t2;
bool lapover = false;
int checkpoint = 0;

std::vector< WOPhysxBox* > projectiles;
std::vector< WOPhysxSphere* > asteroids;
int projectileToDelete = -1;
int asteroidToDelete = -1;

class laserContactEvent : public physx::PxSimulationEventCallback
{
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
    {

    }

    void onWake(physx::PxActor** actors, physx::PxU32 count)
    {

    }

    void onSleep(physx::PxActor** actors, physx::PxU32 count) {}

    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {}

    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}

    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
    {
        std::string explosionSound(ManagerEnvironmentConfiguration::getLMM() + "/sounds/explosion.ogg");
        irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
        irrklang::ISound* sound = soundEngine->play2D(explosionSound.c_str());
        for (physx::PxU32 i = 0; i < nbPairs; i++)
        {
            const physx::PxContactPair& cp = pairs[i];

            if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
            {
                for (int x = 0; x < projectiles.size(); ++x)
                {
                    if ((pairHeader.actors[0] == projectiles[x]->actor) ||
                        (pairHeader.actors[1] == projectiles[x]->actor))
                    {
                        physx::PxActor* otherActor = (projectiles[x]->actor == pairHeader.actors[0]) ?
                            pairHeader.actors[1] : pairHeader.actors[0];

                        for (int y = 0; y < asteroids.size(); ++y)
                        {
                            if (asteroids[y]->actor == otherActor)
                            {
                                asteroidToDelete = y;
                                projectileToDelete = x;
                                return;
                            }
                        }
                    }
                }
                
            }
        }
    }
};




class WOWayPointSphericalDerived : public WOWayPointSpherical {
public:
    void onTrigger() {
        checkpoint++;
        if (checkpoint == 5) {
            t2 = std::chrono::steady_clock::now();
            std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
            std::cout << time_span << std::endl;
            lapover = true;
        }
           std::string checkpointSound(ManagerEnvironmentConfiguration::getLMM() + "/sounds/checkpoint-sound.ogg");
           irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
           irrklang::ISound* sound = soundEngine->play2D(checkpointSound.c_str());

    }
    WOWayPointSphericalDerived(const WayPointParametersBase& params, float radius) : IFace(this), WOWayPointSpherical(params, radius) {
        ;
    }
public:
    static WOWayPointSphericalDerived* New(const WayPointParametersBase& params, float radius) {
        WOWayPointSphericalDerived* ptr = new WOWayPointSphericalDerived(params, radius);
        ptr->onCreate();
        return ptr;
    }
};




GLViewFinalProject* GLViewFinalProject::New( const std::vector< std::string >& args )
{
   GLViewFinalProject* glv = new GLViewFinalProject( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}

physx::PxFilterFlags contactReportFilterShader(
    physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
    physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
    physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
    // let triggers through
    if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
    {
        pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
        return physx::PxFilterFlag::eDEFAULT;
    }
    // generate contacts for all that were not filtered above
    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

    // trigger the contact callback for pairs (A,B) where
    // the filtermask of A contains the ID of B and vice versa.
    if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
        pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

    return physx::PxFilterFlag::eDEFAULT;
}

GLViewFinalProject::GLViewFinalProject( const std::vector< std::string >& args ) : GLView( args )
{
    physx_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, alloc, errorcall);
    physx::PxTolerancesScale ts;
    ts.length = 1;
    ts.speed = 5;
    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *physx_foundation, ts, false);
    physx::PxSceneDesc sc(physics->getTolerancesScale());
    sc.filterShader = physx::PxDefaultSimulationFilterShader;
    sc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sc.kineKineFilteringMode = physx::PxPairFilteringMode::eKEEP;
    sc.filterShader = contactReportFilterShader;


    physics_scene = physics->createScene(sc);
    physics_scene->setFlag(physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
}

void GLViewFinalProject::onCreate()
{
#ifdef VR
    XrFormFactor form_factor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

    XrViewConfigurationType view_type = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    XrReferenceSpaceType play_space_type = XR_REFERENCE_SPACE_TYPE_LOCAL;
    play_space = XR_NULL_HANDLE;

    xrInstance = XR_NULL_HANDLE;
    XrSystemId system_id = XR_NULL_SYSTEM_ID;
    xrSession = XR_NULL_HANDLE;

    viewconfig_views = NULL;

    const char* enabled_exts[1] = { XR_KHR_OPENGL_ENABLE_EXTENSION_NAME };

    XrInstanceCreateInfo instance_create_info = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .next = NULL,
        .createFlags = 0,
        .applicationInfo =
            {
            .applicationName = "Space Race",
            .applicationVersion = 1,
            .engineName = "AftrBurner",
            .engineVersion = 0,
            .apiVersion = XR_CURRENT_API_VERSION,
        },
        .enabledApiLayerCount = 0,
        .enabledApiLayerNames = NULL,
        .enabledExtensionCount = 1,
        .enabledExtensionNames = enabled_exts,
    };

    XrResult result = xrCreateInstance(&instance_create_info, &xrInstance);

    std::cout << "CREATE INSTANCE RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;

    XrSystemGetInfo system_get_info = {
        .type = XR_TYPE_SYSTEM_GET_INFO, .next = NULL, .formFactor = form_factor };

    result = xrGetSystem(xrInstance, &system_get_info, &system_id);

    std::cout << "GET SYSTEM INFO RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;

    // Check graphics requirements
    XrGraphicsRequirementsOpenGLKHR opengl_reqs = { .type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR,
                                                   .next = NULL };
   

    PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
    xrGetInstanceProcAddr(xrInstance, "xrGetOpenGLGraphicsRequirementsKHR",
        reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLGraphicsRequirementsKHR));

    result = pfnGetOpenGLGraphicsRequirementsKHR(xrInstance, system_id, &opengl_reqs);

    std::cout << "GET GRAPHICS REQUIREMENTS RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(ManagerWindowing::getCurrentWindow(), &info) < 0)
    {
        std::cout << "GETTING INFO FAILED" << std::endl;
    }

    uint32_t view_count = 0;
    result = xrEnumerateViewConfigurationViews( xrInstance, system_id, view_type, 0, &view_count, NULL );

    std::cout << "GET VIEW COUNT RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;
    std::cout << "VIEW COUNT: " << view_count << std::endl;

    viewconfig_views = new XrViewConfigurationView[view_count];
    for (uint32_t i = 0; i < view_count; i++) {
        viewconfig_views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
        viewconfig_views[i].next = NULL;
    }

    result = xrEnumerateViewConfigurationViews(xrInstance, system_id, view_type, view_count,
        &view_count, viewconfig_views);

    std::cout << "GET VIEW CONFIGURATIONS RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;

    std::cout << viewconfig_views[0].recommendedImageRectHeight << " " << viewconfig_views[0].recommendedImageRectWidth << std::endl;

    HWND hwnd = info.info.win.window;    

    HDC handle = GetDC(hwnd);
    HGLRC context = wglGetCurrentContext();

    std::cout << "IS WINDOW?: " << IsWindow(hwnd) << std::endl;

    // Create graphics binding
    XrGraphicsBindingOpenGLWin32KHR graphics_binding_gl = {
        .type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR,
        .next = NULL,
        .hDC = handle,
        .hGLRC = context
    };
    
    
    XrSessionCreateInfo session_create_info = {
        .type = XR_TYPE_SESSION_CREATE_INFO, .next = &graphics_binding_gl, .systemId = system_id };

    result = xrCreateSession(xrInstance, &session_create_info, &xrSession);

    std::cout << "CREATE SESSION RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;

    XrPosef identity_pose = { .orientation = {.x = 0, .y = 0, .z = 0, .w = 1},
                                .position = {.x = 0, .y = 0, .z = -5} };

    XrReferenceSpaceCreateInfo play_space_create_info = { .type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO,
                                                         .next = NULL,
                                                         .referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL,
                                                         //.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE,
                                                         .poseInReferenceSpace = identity_pose };

    result = xrCreateReferenceSpace(xrSession, &play_space_create_info, &play_space);

    std::cout << "CREATE PLAYSPACE RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << " " << result << std::endl;
    //ManagerEnvironmentConfiguration::registerVariable("stereoseparation", "0.05f");
#endif

   if( this->pe != NULL )
   {
      //optionally, change gravity direction and magnitude here
      //The user could load these values from the module's aftr.conf
      this->pe->setGravityNormalizedVector( Vector( 0,0,-1.0f ) );
      this->pe->setGravityScalar( Aftr::GRAVITY );
   }
   this->setActorChaseType( STANDARDEZNAV ); //Default is STANDARDEZNAV mode
   //this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1

}


GLViewFinalProject::~GLViewFinalProject()
{
}

void GLViewFinalProject::updateWorld()
{
   GLView::updateWorld(); //Just call the parent's update world first.
                          //If you want to add additional functionality, do it after
                          //this call.

   if (rb) {
       camera->addForce(physx::PxVec3(cam->getLookDirection().x, cam->getLookDirection().y, cam->getLookDirection().z), physx::PxForceMode::eVELOCITY_CHANGE);
       //this->cam->moveInLookDirection(this->cam->getCameraVelocity());
   }
   if (lb) {
       camera->addForce(camera->getLinearVelocity() * -0.1, physx::PxForceMode::eVELOCITY_CHANGE);
       //this->cam->moveOppositeLookDirection(this->cam->getCameraVelocity());
   }
   if (dup) {
       cam->rotateAboutRelY(-.02);
   }
   if (ddown) {
       cam->rotateAboutRelY(.02);
   }
   if (dleft) {
       cam->rotateAboutRelZ(.02);
   }
   if (dright) {
       cam->rotateAboutRelZ(-.02);
   }
   if (rollLeft) {
       cam->rotateAboutRelX(-.02);
   }
   if (rollRight) {
       cam->rotateAboutRelX(.02);
   }
   if (!start && !lapover) {
       if (this->timerlabel != NULL) {
           t2 = std::chrono::steady_clock::now();
           std::string curlaptime = std::to_string(std::chrono::duration<double>(t2 - t1).count()) + 's';
           this->timerlabel->setText(curlaptime);
       }
   }
   if (lapover) {
       this->soundEngine->stopAllSounds();
}
#ifdef VR
   // Check current state of openxr
   XrEventDataBuffer runtime_event = { .type = XR_TYPE_EVENT_DATA_BUFFER, .next = NULL };
   XrResult poll_result = xrPollEvent( xrInstance, &runtime_event);

   if (poll_result == XR_SUCCESS)
   {
       switch (runtime_event.type)
       {
       case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
           XrEventDataSessionStateChanged* event = (XrEventDataSessionStateChanged*)&runtime_event;
           switch (event->state)
           {
           case XR_SESSION_STATE_READY:
               if (!sessionRunning) {
                   XrSessionBeginInfo session_begin_info = {
                       .type = XR_TYPE_SESSION_BEGIN_INFO,
                       .next = NULL,
                       .primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO };
                   XrResult result = xrBeginSession(xrSession, &session_begin_info);

                   std::cout << "BEGIN SESSION RESULT: " << ((result == XR_SUCCESS) ? "succeed" : "fail") << std::endl;

                   delete glRenderer;
                   glRenderer = VRRenderer::New(xrSession, viewconfig_views, play_space);
               }
               break;
           default:
               std::cout << "SOME OTHER STATE CHANGE " << event->state << std::endl;
           }
       }
       default:
           std::cout << "SOME OTHER THING? " << runtime_event.type << std::endl;
           break;
       }
   }
   else if (poll_result != XR_EVENT_UNAVAILABLE)
   {
       std::cout << "FAILED TO POLL " << poll_result << std::endl;
   }
#endif

   //std::cout << cam->getPosition() << std::endl;

   // Handle physics simulations
   if (firstTime || (physics_scene != nullptr && physics_scene->fetchResults(true)))
   {
       physics_scene->simulate(ManagerSDLTime::getTimeSinceLastMainLoopIteration() / 1000.0f);
       firstTime = false;
   }

   if (physics_controls)
   {
       cam->setPosition(camera->getGlobalPose().p.x, camera->getGlobalPose().p.y, camera->getGlobalPose().p.z);
   }

   // Move lasers forward
   for (int i = 0; i < projectiles.size(); ++i)
   {
       //projectiles[i]->moveRelative(projectiles[i]->getLookDirection());
       //projectiles[i]->setPosition( projectiles[i]->getPosition() + cam->getLookDirection());
   }

   if (projectileToDelete != -1 && asteroidToDelete != -1)
   {
       breakAsteroid();
   }
}


void GLViewFinalProject::onResizeWindow( GLsizei width, GLsizei height )
{
   GLView::onResizeWindow( width, height ); //call parent's resize method.
}


void GLViewFinalProject::onMouseDown( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseDown( e );
}


void GLViewFinalProject::onMouseUp( const SDL_MouseButtonEvent& e )
{
   GLView::onMouseUp( e );
}


void GLViewFinalProject::onMouseMove( const SDL_MouseMotionEvent& e )
{
   GLView::onMouseMove( e );
}


void GLViewFinalProject::onKeyDown( const SDL_KeyboardEvent& key )
{
   GLView::onKeyDown( key );
   if( key.keysym.sym == SDLK_0 )
      this->setNumPhysicsStepsPerRender( 1 );

   if( key.keysym.sym == SDLK_1 )
   {
       //this->cam->moveInLookDirection(this->cam->getCameraVelocity());
       camera->addForce( physx::PxVec3( cam->getLookDirection().x, cam->getLookDirection().y, cam->getLookDirection().z ), physx::PxForceMode::eVELOCITY_CHANGE );
       if (start) {
           t1 = std::chrono::steady_clock::now();
           start = false;
       }
   }
   else if (key.keysym.sym == SDLK_2)
   {
       //this->cam->moveOppositeLookDirection(this->cam->getCameraVelocity());
       //camera->addForce(physx::PxVec3(-1 * cam->getLookDirection().x, -1 * cam->getLookDirection().y, -1 * cam->getLookDirection().z), physx::PxForceMode::eVELOCITY_CHANGE);
       camera->addForce(camera->getLinearVelocity() * -0.1, physx::PxForceMode::eVELOCITY_CHANGE);
       
       /*if (!start) {
           t2 = std::chrono::steady_clock::now();
           std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
           std::cout << time_span << std::endl;
           lapover = true;
       }*/
   }
   else if (key.keysym.sym == SDLK_3)
   {
       fireLaser();
   }
   else if (key.keysym.sym == SDLK_p)
   {
       std::cout << "Current Pos: " << cam->getPosition() << std::endl;
       std::cout << cam->getPose() << std::endl;
   }
   else if (key.keysym.sym == SDLK_d)
   {
       physics_controls = !physics_controls;
   }
}


void GLViewFinalProject::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


void GLViewFinalProject::onJoyButtonDown(const SDL_JoyButtonEvent& key)
{
    GLView::onJoyButtonDown(key);
    //whichbutton = key;
    if (key.type == SDL_JOYBUTTONUP) {
        std::cout << "released\n";
    }
    
    if (key.button == 4)
    {
        lb = true;
        std::cout << "LB" << std::endl;        
    }
    if (key.button == 5)
    {
        rb = true;
        if (start) {
            t1 = std::chrono::steady_clock::now();
            start = false;
        }
    }

    if (key.button == 8) {
        rollLeft = true;
        //cam->rotateAboutRelX(-.2);
    }
    if (key.button == 9) {
        rollRight = true;
        //cam->rotateAboutRelX(.2);
    }
    if (key.button == 0) {
        fireLaser();
    }

}

void GLViewFinalProject::onJoyButtonUp(const SDL_JoyButtonEvent& key)
{
    GLView::onJoyButtonUp(key);
    //whichbutton = key;
    if (key.type == SDL_JOYBUTTONUP) {
        std::cout << "released\n";
    }
    if (key.button == 4)
    {
        lb = false;
        std::cout << "LB" << std::endl;
    }
    if (key.button == 5)
    {
        rb = false;
        std::cout << "RB" << std::endl;
    }
    if (key.button == 8) {
        rollLeft = false;
    }
    if (key.button == 9) {
        rollRight = false;
    }
}

void GLViewFinalProject::onJoyHatMotion(const SDL_JoyHatEvent& key) {
    GLView::onJoyHatMotion(key);
    if (key.value == 1) {
        //cam->rotateAboutRelY(-.2);
        dup = true;
    }
    if (key.value == 4) {
        //cam->rotateAboutRelY(.2);
        ddown = true;
    }
    if (key.value == 8) {
        //cam->rotateAboutRelZ(.2);
        dleft = true;
    }
    if (key.value == 2) {
        //cam->rotateAboutRelZ(-.2);
        dright = true;
    }
    if (key.value == 0) {
        dup = false;
        ddown = false;
        dleft = false;
        dright = false;
    }
}

void GLViewFinalProject::setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
    physx::PxFilterData filterData;
    filterData.word0 = filterGroup; // word0 = own ID
    filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback;
    const physx::PxU32 numShapes = actor->getNbShapes();
    physx::PxShape** shapes = new physx::PxShape * [numShapes];
    actor->getShapes(shapes, numShapes);
    for (physx::PxU32 i = 0; i < numShapes; i++)
    {
        physx::PxShape* shape = shapes[i];
        shape->setSimulationFilterData(filterData);
    }
    delete shapes;
}

void GLViewFinalProject::fireLaser()
{

    std::string laserSound(ManagerEnvironmentConfiguration::getLMM() + "/sounds/SpaceLaserShot.ogg");
    irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
    irrklang::ISound* sound = soundEngine->play2D(laserSound.c_str());
    {
        //WOPhysxSphere* wo = WOPhysxSphere::New(ManagerEnvironmentConfiguration::getLMM() + "/models/laser/my laser.blend", Vector(1, 1, 1), physics, physics_scene, cam->getPosition() + cam->getLookDirection() * 5);
        WOPhysxBox* wo = WOPhysxBox::New(ManagerEnvironmentConfiguration::getLMM() + "/models/laser/mylaser.obj", Vector(0.5, 0.5, 0.5), physics, physics_scene, cam, camera );
        wo->upon_async_model_loaded([this, wo]() {
            setupFiltering(wo->actor, FILTER::LASER, FILTER::ASTEROID);

            });
        //wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        worldLst->push_back(wo);
        projectiles.push_back(wo);
    }
}

void GLViewFinalProject::breakAsteroid()
{
    worldLst->eraseViaWOptr(asteroids[asteroidToDelete]);
    worldLst->eraseViaWOptr(projectiles[projectileToDelete]);

    //physx::PxVec3 v = projectiles[projectileToDelete]->actor->getLinearVelocity();

    physics_scene->removeActor(*asteroids[asteroidToDelete]->actor);
    physics_scene->removeActor(*projectiles[projectileToDelete]->actor);


    //projectiles[projectileToDelete]->actor->clearForce();
    //[projectileToDelete]->actor->addForce({ projectiles[projectileToDelete]->getLookDirection().x, projectiles[projectileToDelete]->getLookDirection().y, projectiles[projectileToDelete]->getLookDirection().z });


    for (int i = 0; i < 4; ++i)
    {
        Vector force( ((rand() % 1000) - 500)/1000.0, ((rand() % 1000) - 500) / 1000.0, ((rand() % 1000) - 500) / 1000.0 );
        //Vector subPosition = asteroids[asteroidToDelete]->getPosition() + (directions[i] * 20);
        //std::cout << "position: " << subPosition << std::endl;
        WOPhysxSphere* wo = WOPhysxSphere::New(ManagerEnvironmentConfiguration::getLMM() + "/models/asteroid/10464_Asteroid_v1_Iterations-2.obj", { 0.0005f, 0.0005f, 0.0005f }, physics, physics_scene, asteroids[asteroidToDelete]->getPosition() + force * 5);
        wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
        wo->upon_async_model_loaded([this, force, wo]()
            {
                setupFiltering(wo->actor, FILTER::ASTEROID, FILTER::LASER);
                wo->actor->addForce(physx::PxVec3(force.x, force.y, force.z) * 100, physx::PxForceMode::eVELOCITY_CHANGE);
            });
        worldLst->push_back(wo);
        asteroids.push_back(wo);
    }

    projectileToDelete = -1;
    asteroidToDelete = -1;
}

/*void GLViewFinalProject::onControllerAxisMotion(const SDL_ControllerAxisEvent& joy)
{
    std::cout << "  0x" << std::hex << (int)joy.type << std::dec << ",   " << (int)joy.type << "\n";
    std::cout << "GLView::onControllerAxisMotion" << (int)joy.which << "   axis:" << (int)joy.axis << "   val:" << joy.value << "\n";
}
void GLViewFinalProject::onJoyBallMotion(const SDL_JoyBallEvent& joy)
{
    GLView::onJoyBallMotion(joy);

}
void GLViewFinalProject::onJoyHatMotion(const SDL_JoyHatEvent& joy)
{
    GLView::onJoyHatMotion(joy);

}

void GLViewFinalProject::onControllerButtonDown(const SDL_ControllerButtonEvent& button) {
    std::cout << "GLView::onControllerButtonDown" << (int)button.which << "   but:" << (int)button.button << "   state:PRESSED\n";
}*/

void Aftr::GLViewFinalProject::loadMap()
{
   this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
   this->actorLst = new WorldList();
   this->netLst = new WorldList();

   ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
   ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
   ManagerOpenGLState::enableFrustumCulling = false;
   Axes::isVisible = true;
   this->glRenderer->isUsingShadowMapping( false ); //set to TRUE to enable shadow mapping, must be using GL 3.2+

   this->cam->setPosition( 15,15,10 );

   std::string shinyRedPlasticCube( ManagerEnvironmentConfiguration::getSMM() + "/models/cube4x4x4redShinyPlastic_pp.wrl" );
   std::string wheeledCar( ManagerEnvironmentConfiguration::getSMM() + "/models/rcx_treads.wrl" );
   std::string grass( ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl" );
   std::string human( ManagerEnvironmentConfiguration::getSMM() + "/models/human_chest.wrl" );
   
   //SkyBox Textures readily available
   std::vector< std::string > skyBoxImageNames; //vector to store texture paths
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg" );

   {
      //Create a light
      float ga = 0.1f; //Global Ambient Light level for this module
      ManagerLight::setGlobalAmbientLight( aftrColor4f( ga, ga, ga, 1.0f ) );
      WOLight* light = WOLight::New();
      light->isDirectionalLight( true );
      light->setPosition( Vector( 0, 0, 100 ) );
      //Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
      //for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
      light->getModel()->setDisplayMatrix( Mat4::rotateIdentityMat( { 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD ) );
      light->setLabel( "Light" );
      worldLst->push_back( light );
   }

   {
      //Create the SkyBox
      WO* wo = WOSkyBox::New( skyBoxImageNames.at( 0 ), this->getCameraPtrPtr() );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->setLabel( "Sky Box" );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      worldLst->push_back( wo );
   }

   std::string cockpit(ManagerEnvironmentConfiguration::getLMM() + "/models/spaceship-cockpit/Spaceship_Cockpit.fbx");
   std::string ring(ManagerEnvironmentConfiguration::getLMM() + "/models/66-ring-ornament/vers(4)-men-design #RGmen_US 5_H 1.65_W 5.stl");

   /*SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
   xcontroller = nullptr; 
   if (SDL_IsGameController(0)) {
       xcontroller = SDL_GameControllerOpen(0);
   }
   //SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
   {
       fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
       exit(1);
   }
   //SDL_GameController* xcontroller;
   SDL_Joystick* joy;
   // Check for joystick
   /*if (SDL_NumJoysticks() > 0) {
       // Open joystick
       joy = SDL_JoystickOpen(0);

       if (joy)
       {
           printf("game controller? %d\n", SDL_IsGameController(0));
           printf("Opened Joystick 0\n");
           printf("Name: %s\n", SDL_JoystickName(0));
           printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
           printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
           printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
           printf("Instance ID: %d\n", SDL_JoystickInstanceID(joy));
           printf("controller: %d\n", SDL_GameControllerFromInstanceID(SDL_JoystickInstanceID(joy)));
           xcontroller = SDL_GameControllerFromInstanceID(SDL_JoystickInstanceID(joy));
           printf("%s\n", SDL_GameControllerGetBindForButton(xcontroller, SDL_CONTROLLER_BUTTON_PADDLE4));
       }
       else
           printf("Couldn't open Joystick 0\n");

   }


   printf("%i joysticks were found.\n\n", SDL_NumJoysticks());
   printf("The names of the joysticks are:\n");

   

   for (int i = 0; i < SDL_NumJoysticks(); i++)
   {
       printf("    %s\n", SDL_JoystickName(0));
   }*/
   /*
   { 
      ////Create the infinite grass plane (the floor)
      WO* wo = WO::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
      wo->setPosition( Vector( 0, 0, 0 ) );
      wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
      wo->upon_async_model_loaded( [wo]()
         {
            ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
            grassSkin.getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
            grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
            grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
            grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
            grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
         } );
      wo->setLabel( "Grass" );
      worldLst->push_back( wo );
   }*/
   this->timerlabel = WOGUILabel::New(NULL);
   this->timerlabel->setText("0.00s");
   this->timerlabel->setLabel("guiLabelTimer");
   this->timerlabel->setPosition(Vector(0.9f, 0.9f, 1.0f));
   this->timerlabel->setColor(255, 255, 255, 255);
   this->timerlabel->setFontSize(16);
   worldLst->push_back(timerlabel);

   {
       ////Create new WO
       WO* wo = WO::New(cockpit, Vector(.01, .01, .01), MESH_SHADING_TYPE::mstFLAT);
       wo->setPosition(Vector(0, 0, 12));
       wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       wo->upon_async_model_loaded([wo]()
           {
               ModelMeshSkin& cubeskin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
               //cubeskin.getMultiTextureSet().at(0)->setTextureRepeats(5.0f);
               //cubeskin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
               //cubeskin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
               //cubeskin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
               //cubeskin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
           });
       wo->setLabel("cockpit");
       worldLst->push_back(wo);



       WO* check1 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check1->setPosition(Vector(100, 0, 0));
       check1->rotateAboutRelY(1.57);
       check1->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check1->upon_async_model_loaded([check1]()
           {
               ModelMeshSkin& cubeskin = check1->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
               //cubeskin.getMultiTextureSet().at(0)->setTextureRepeats(5.0f);
               //cubeskin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
               //cubeskin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
               //cubeskin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
               //cubeskin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
           });
       check1->setLabel("checkpoint");
       worldLst->push_back(check1);


       WO* check2 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check2->setPosition(Vector(250, 0, 40));
       check2->rotateAboutRelY(1.57);
       check2->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check2->upon_async_model_loaded([check2]()
           {
               ModelMeshSkin& cubeskin = check2->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
               
           });
       check2->setLabel("checkpoint");
       worldLst->push_back(check2);

       WO* check3 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check3->setPosition(Vector(400, 0, 80));
       check3->rotateAboutRelY(1.57);
       check3->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check3->upon_async_model_loaded([check3]()
           {
               ModelMeshSkin& cubeskin = check3->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);

           });
       check3->setLabel("checkpoint");
       worldLst->push_back(check3);

       WO* check4 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check4->setPosition(Vector(525, 40, 100));
       check4->rotateAboutRelY(1.57);
       check4->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check4->upon_async_model_loaded([check4]()
           {
               ModelMeshSkin& cubeskin = check4->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);

           });
       check4->setLabel("checkpoint");
       worldLst->push_back(check4);

       WO* check5 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check5->setPosition(Vector(650, 80, 100));
       check5->rotateAboutRelY(1.57);
       check5->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check5->upon_async_model_loaded([check5]()
           {
               ModelMeshSkin& cubeskin = check5->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);

           });
       check5->setLabel("checkpoint");
       worldLst->push_back(check5);

   }
   std::string music(ManagerEnvironmentConfiguration::getLMM() + "/sounds/music_astro_race.ogg");
   this->soundEngine = irrklang::createIrrKlangDevice();
   irrklang::ISound* sound = soundEngine->play2D(music.c_str());

   std::vector< Aftr::Vector > obstacle_positions = { {175, 1, 12}, {325, -2, 60}, {466, 22, 80}, {578, 62, 90}, {700, 82, 101}, {772, 74, 94}, {900, 52, 100} };
   std::vector< Aftr::Vector > obstacle_scales = { {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f}, {0.005f, 0.005f, 0.005f} };

   for ( int i = 0; i < obstacle_positions.size(); ++i )
   {
       WOPhysxSphere* wo = WOPhysxSphere::New(ManagerEnvironmentConfiguration::getLMM() + "/models/asteroid/10464_Asteroid_v1_Iterations-2.obj", obstacle_scales[i], physics, physics_scene, obstacle_positions[i]);
       wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       wo->upon_async_model_loaded([this, wo]()
           {
               setupFiltering(wo->actor, FILTER::ASTEROID, FILTER::LASER);
           });
       worldLst->push_back(wo);
       asteroids.push_back(wo);
   }

   {
       physx::PxMaterial* gMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
       physx::PxShape* shape = physics->createShape(physx::PxSphereGeometry(1), *gMaterial, true);

       physx::PxTransform t({ cam->getPosition().x, cam->getPosition().y, cam->getPosition().z});

       camera = physics->createRigidDynamic(t);
       camera->attachShape(*shape);

       physics_scene->addActor(*camera);
   }

   laserContactEvent* callback = new laserContactEvent;
   physics_scene->setSimulationEventCallback(callback);

   //{
   //    //Create a model of earth
   //    WO* wo = WO::New(ManagerEnvironmentConfiguration::getSMM() + "/models/sphereR5Earth.wrl", Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
   //    wo->setPosition(-3, -3, 5);
   //    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //    wo->setLabel("Earth");
   //    worldLst->push_back(wo);
   //}


   //{ 
   //   ////Create the infinite grass plane (the floor)
   //   WO* wo = WO::New( grass, Vector( 3, 3, 3 ), MESH_SHADING_TYPE::mstFLAT );
   //   wo->setPosition( Vector( 0, 0, 0 ) );
   //   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //   wo->upon_async_model_loaded( [wo]()
   //      {
   //         ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 );
   //         grassSkin.getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
   //         grassSkin.setAmbient( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Color of object when it is not in any light
   //         grassSkin.setDiffuse( aftrColor4f( 1.0f, 1.0f, 1.0f, 1.0f ) ); //Diffuse color components (ie, matte shading color of this object)
   //         grassSkin.setSpecular( aftrColor4f( 0.4f, 0.4f, 0.4f, 1.0f ) ); //Specular color component (ie, how "shiney" it is)
   //         grassSkin.setSpecularCoefficient( 10 ); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
   //      } );
   //   wo->setLabel( "Grass" );
   //   worldLst->push_back( wo );
   //}

   // Create the floor in the physics scene
   /*{
       physx::PxMaterial* gMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
       physx::PxRigidStatic* groundPlane = PxCreatePlane(*physics, physx::PxPlane(0, 0, 1, 0), *gMaterial);
       physics_scene->addActor(*groundPlane);
   
   }*/

   //physics_scene->setGravity(physx::PxVec3(0, 0, -5));

   // Turn off axes
   Axes::isVisible = false;

   createFinalProjectWayPoints();
}


void GLViewFinalProject::createFinalProjectWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 0;
   params.useCamera = true;
   params.visible = false;
   WOWayPointSpherical* wayPt = WOWayPointSphericalDerived::New( params, 8 );
   wayPt->setPosition( Vector( 100, 0, 0 ) );
   worldLst->push_back( wayPt );

   WOWayPointSpherical* wayPt2 = WOWayPointSphericalDerived::New(params, 8);
   wayPt2->setPosition(Vector(250, 0, 40));
   worldLst->push_back(wayPt2);

   WOWayPointSpherical* wayPt3 = WOWayPointSphericalDerived::New(params, 8);
   wayPt3->setPosition(Vector(400, 0, 80));
   worldLst->push_back(wayPt3);

   WOWayPointSpherical* wayPt4 = WOWayPointSphericalDerived::New(params, 8);
   wayPt4->setPosition(Vector(525, 40, 100));
   worldLst->push_back(wayPt4);

   WOWayPointSpherical* wayPt5 = WOWayPointSphericalDerived::New(params, 8);
   wayPt5->setPosition(Vector(650, 80, 100));
   worldLst->push_back(wayPt5);
}
