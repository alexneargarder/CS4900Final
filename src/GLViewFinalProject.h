#pragma once

#include "GLView.h"

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
   //virtual void onControllerAxisMotion(const SDL_ControllerAxisEvent& joy);
   //virtual void onJoyHatMotion(const SDL_JoyHatEvent& joy);
   //virtual void onJoyBallMotion(const SDL_JoyBallEvent& joy);
   //virtual void onControllerButtonDown(const SDL_ControllerButtonEvent& button);




protected:
   GLViewFinalProject( const std::vector< std::string >& args );
   virtual void onCreate();  
   WOGUILabel* timerlabel;

};

} //namespace Aftr
