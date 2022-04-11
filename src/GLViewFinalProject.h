#pragma once

#include "GLView.h"
// OpenXR
#include "openxr.h"
#include "openxr_platform.h"

namespace Aftr
{
   class Camera;

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

protected:
   GLViewFinalProject( const std::vector< std::string >& args );
   virtual void onCreate();

   XrInstance xrInstace;
   XrSession xrSession;
};

} //namespace Aftr
