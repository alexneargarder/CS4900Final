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

//SDL_JoyButtonEvent whichbutton;
bool rb = false;
bool lb = false;
//SDL_GameController* xcontroller;
//SDL_Event ev;
using namespace Aftr;
bool start = true;
std::chrono::steady_clock::time_point t1;
std::chrono::steady_clock::time_point t2;
bool lapover = false;
int checkpoint = 0;





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



GLViewFinalProject::GLViewFinalProject( const std::vector< std::string >& args ) : GLView( args )
{
   
}


void GLViewFinalProject::onCreate()
{

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
       this->cam->moveInLookDirection(this->cam->getCameraVelocity());
   }
   if (lb) {
       this->cam->moveOppositeLookDirection(this->cam->getCameraVelocity());
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
       this->cam->moveInLookDirection(this->cam->getCameraVelocity());
       if (start) {
           t1 = std::chrono::steady_clock::now();
           start = false;
       }
   }
   if (key.keysym.sym == SDLK_2)
   {
       this->cam->moveOppositeLookDirection(this->cam->getCameraVelocity());
       /*if (!start) {
           t2 = std::chrono::steady_clock::now();
           std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
           std::cout << time_span << std::endl;
           lapover = true;
       }*/
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
        std::cout << "RB" << std::endl;
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
       check1->setPosition(Vector(0, 0, 40));
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
       check2->setPosition(Vector(-50, 0, 40));
       check2->rotateAboutRelY(1.57);
       check2->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check2->upon_async_model_loaded([check2]()
           {
               ModelMeshSkin& cubeskin = check2->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
               
           });
       check2->setLabel("checkpoint");
       worldLst->push_back(check2);

       WO* check3 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check3->setPosition(Vector(-150, 0, 80));
       check3->rotateAboutRelY(1.57);
       check3->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check3->upon_async_model_loaded([check3]()
           {
               ModelMeshSkin& cubeskin = check3->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);

           });
       check3->setLabel("checkpoint");
       worldLst->push_back(check3);

       WO* check4 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check4->setPosition(Vector(-250, 40, 100));
       check4->rotateAboutRelY(1.57);
       check4->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
       check4->upon_async_model_loaded([check4]()
           {
               ModelMeshSkin& cubeskin = check4->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);

           });
       check4->setLabel("checkpoint");
       worldLst->push_back(check4);

       WO* check5 = WO::New(ring, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
       check5->setPosition(Vector(-300, 80, 100));
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
   wayPt->setPosition( Vector( 0, 0, 40 ) );
   worldLst->push_back( wayPt );

   WOWayPointSpherical* wayPt2 = WOWayPointSphericalDerived::New(params, 8);
   wayPt2->setPosition(Vector(-50, 0, 40));
   worldLst->push_back(wayPt2);

   WOWayPointSpherical* wayPt3 = WOWayPointSphericalDerived::New(params, 8);
   wayPt3->setPosition(Vector(-150, 0, 80));
   worldLst->push_back(wayPt3);

   WOWayPointSpherical* wayPt4 = WOWayPointSphericalDerived::New(params, 8);
   wayPt4->setPosition(Vector(-250, 40, 100));
   worldLst->push_back(wayPt4);

   WOWayPointSpherical* wayPt5 = WOWayPointSphericalDerived::New(params, 8);
   wayPt5->setPosition(Vector(-300, 80, 100));
   worldLst->push_back(wayPt5);
}
