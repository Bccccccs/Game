#pragma once
#include "cocos2d.h"

class GameController;

class GameScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene(int levelIndex);
    virtual bool init() override;
    CREATE_FUNC(GameScene);

    void setLevelIndex(int levelIndex);

protected:
    void onExit() override;

private:
    int _levelIndex = 1;
    GameController* _controller = nullptr;
};
