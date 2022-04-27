#pragma once

#include "GLView.h"
#include "irrKlang.h"

#ifdef VR
// OpenXR
#define XR_USE_GRAPHICS_API_OPENGL
#define XR_USE_PLATFORM_WIN32
#include "windows.h"
#include "openxr.h"
#include "openxr_platform.h"
#endif

#include "PxPhysicsAPI.h"
#include "vehicle/PxVehicleSDK.h"
#include "extensions/PxDefaultErrorCallback.h"
#include "extensions/PxDefaultAllocator.h"

#include "WOPhysxBox.h"

namespace Aftr
{
   class Camera;
   class WOGUILabel;

class GLViewFinalProject : public GLView
{
public:
   static GLViewFinalProject* New( const std::vector< std::string >& outArgs );
   virtual ~GLViewFinalProject();
   virtual void updateWorld(); ///< Called once per frame
   virtual void loadMap(); ///< Called once at startup to build this module's scene
   virtual void createFinalProjectWayPoints();
   virtual void onResizeWindow( GLsizei width, GLsizei height );
   virtual void onMouseDown( const SDL_MouseButtonEvent& e );
   virtual void onMouseUp( const SDL_MouseButtonEvent& e );
   virtual void onMouseMove( const SDL_MouseMotionEvent& e );
   virtual void onKeyDown( const SDL_KeyboardEvent& key );
   virtual void onKeyUp( const SDL_KeyboardEvent& key );
   virtual void onJoyButtonDown(const SDL_JoyButtonEvent& key);
   virtual void onJoyButtonUp(const SDL_JoyButtonEvent& key);
   irrklang::ISoundEngine* soundEngine;
   //virtual void onControllerAxisMotion(const SDL_ControllerAxisEvent& joy);
   //virtual void onJoyHatMotion(const SDL_JoyHatEvent& joy);
   //virtual void onJoyBallMotion(const SDL_JoyBallEvent& joy);
   //virtual void onControllerButtonDown(const SDL_ControllerButtonEvent& button);
   void fireLaser();
   void breakAsteroid();
   void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);


protected:

   
   GLViewFinalProject( const std::vector< std::string >& args );

   virtual void onCreate();
   WOGUILabel* timerlabel;

#ifdef VR
   XrInstance xrInstance;
   XrSession xrSession;
   XrSpace play_space;
   XrViewConfigurationView* viewconfig_views;
   XrEventDataBuffer runtime_event;
   bool sessionRunning = false;
#endif

   physx::PxDefaultAllocator alloc;
   physx::PxDefaultErrorCallback errorcall;

   physx::PxFoundation* physx_foundation;
   physx::PxPhysics* physics;
   physx::PxScene* physics_scene;
   bool firstTime = true;
   bool physics_controls = true;

   physx::PxRigidDynamic* camera;

   enum FILTER
   {
       LASER = (1 << 0),
       ASTEROID = (1 << 1),
   };
};


} //namespace Aftr
