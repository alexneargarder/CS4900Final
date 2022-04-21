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

#include "SDL_syswm.h"
#include "AftrOpenGLIncludes.h"
#include "VRRenderer.h"

using namespace Aftr;

GLViewFinalProject* GLViewFinalProject::New( const std::vector< std::string >& args )
{
   GLViewFinalProject* glv = new GLViewFinalProject( args );
   glv->init( Aftr::GRAVITY, Vector( 0, 0, -1.0f ), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE );
   glv->onCreate();
   return glv;
}


GLViewFinalProject::GLViewFinalProject( const std::vector< std::string >& args ) : GLView( args )
{

}

void GLViewFinalProject::onCreate()
{
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

   //std::cout << cam->getPosition() << std::endl;
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

   }
}


void GLViewFinalProject::onKeyUp( const SDL_KeyboardEvent& key )
{
   GLView::onKeyUp( key );
}


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
   skyBoxImageNames.push_back( ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg" );

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
   }

   //{
   //    //Create a model of earth
   //    WO* wo = WO::New(ManagerEnvironmentConfiguration::getSMM() + "/models/sphereR5Earth.wrl", Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
   //    wo->setPosition(-3, -3, 5);
   //    wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
   //    wo->setLabel("Earth");
   //    worldLst->push_back(wo);
   //}

   //createFinalProjectWayPoints();
}


void GLViewFinalProject::createFinalProjectWayPoints()
{
   // Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
   WayPointParametersBase params(this);
   params.frequency = 5000;
   params.useCamera = true;
   params.visible = true;
   WOWayPointSpherical* wayPt = WOWayPointSpherical::New( params, 3 );
   wayPt->setPosition( Vector( 50, 0, 3 ) );
   worldLst->push_back( wayPt );
}
