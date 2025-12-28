#pragma once
#include "cocos2d.h"

class StartScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(StartScene);
};
