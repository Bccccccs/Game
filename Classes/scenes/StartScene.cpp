#include "scenes/StartScene.h"
#include "scenes/LevelSelectScene.h"

USING_NS_CC;

Scene* StartScene::createScene()
{
    return StartScene::create();
}

bool StartScene::init()
{
    if (!Scene::init()) return false;
 
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 背景（先用纯色，后续你再换图）
    auto bg = LayerColor::create(Color4B(18, 26, 44, 255));
    this->addChild(bg);

    auto titlelabel = Label::createWithTTF("C a r d G a m e",
        "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf"
    ,96);
 
    auto titlevisibleSize = Director::getInstance()->getVisibleSize();
    auto titleorigin = Director::getInstance()->getVisibleOrigin();

    titlelabel->setPosition(
        titleorigin.x + titlevisibleSize.width * 0.5f,
        titleorigin.y + titlevisibleSize.height * 0.5f+300
    );
    this->addChild(titlelabel, 1);
    //title
    // “START” 按钮
    auto startLabel = Label::createWithTTF("START", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 72);
    auto startItem = MenuItemLabel::create(startLabel, [](Ref*) {
        auto nextScene = LevelSelectScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.25f, nextScene));
        });

    startItem->setPosition(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.5f);

    auto menu = Menu::create(startItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    return true;
}
