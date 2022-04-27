#pragma once

#include "WOGUILabel.h"


namespace Aftr
{
    class GLViewFinalProject;

class MainMenuButton : public WOGUILabel
{
public:

    static MainMenuButton* New(WOGUI* parentWOGUI, int levelNum, GLViewFinalProject* glview);

    virtual ~MainMenuButton();

    virtual void onMouseClicked(const SDL_MouseButtonEvent& e) override;
    virtual void onMouseEnter(const SDL_MouseMotionEvent& e) override;
    virtual void onMouseLeave(const SDL_MouseMotionEvent& e) override;

protected:
    int levelNum;
    GLViewFinalProject* glview;
    MainMenuButton(WOGUI* parentWOGUI);

    virtual void onCreate(int levelNum, GLViewFinalProject* glview);
};

}