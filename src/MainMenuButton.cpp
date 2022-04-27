#include "MainMenuButton.h"
#include "ModelMeshSkin.h"
#include "MGLGUI.h"
#include "GLViewFinalProject.h"

using namespace Aftr;

MainMenuButton* MainMenuButton::New(WOGUI* parentWOGUI, int levelNum, GLViewFinalProject* glview)
{
    MainMenuButton* btn = new MainMenuButton(parentWOGUI);
    btn->onCreate(levelNum, glview);
    return btn;
}

void MainMenuButton::onCreate(int levelNum, GLViewFinalProject* glview)
{
    WOGUILabel::onCreate(); 

    this->setColor(0, 0, 0, 100);
    this->levelNum = levelNum;
    this->glview = glview;

    //this->setParentWorldObject(this);
}

MainMenuButton::MainMenuButton(WOGUI* parentWOGUI) : IFace( this ), WOGUILabel( parentWOGUI )
{

}

MainMenuButton::~MainMenuButton()
{

}

void MainMenuButton::onMouseClicked(const SDL_MouseButtonEvent& e)
{
    glview->chooseLevel(levelNum);
}

void MainMenuButton::onMouseEnter(const SDL_MouseMotionEvent& e)
{
    this->setColor(0, 0, 0, 255);
}

void MainMenuButton::onMouseLeave(const SDL_MouseMotionEvent& e)
{
    this->setColor(0, 0, 0, 100);
}