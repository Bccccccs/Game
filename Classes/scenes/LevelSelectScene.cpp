#include "scenes/LevelSelectScene.h"
#include "scenes/StartScene.h"
#include "scenes/GameScene.h"

USING_NS_CC;

Scene* LevelSelectScene::createScene()
{
    return LevelSelectScene::create();
}

bool LevelSelectScene::init()
{
    if (!Scene::init()) return false;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto bg = LayerColor::create(Color4B(10, 10, 10, 255));
    this->addChild(bg);

    auto title = Label::createWithTTF("Select Level", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 56);
    title->setPosition(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.78f);
    this->addChild(title, 1);

    // 只放第一关（可点击）
    auto l1Label = Label::createWithTTF("Level 1", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 48);
    auto level1Item = MenuItemLabel::create(l1Label, [](Ref*) {
        auto game = GameScene::createScene(1);
        Director::getInstance()->replaceScene(TransitionFade::create(0.25f, game));
        });

    level1Item->setPosition(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.52f);
    auto l2Label = Label::createWithTTF("Level 2", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 48);
    auto level2Item = MenuItemLabel::create(l2Label, [](Ref*) {
        auto game = GameScene::createScene(1);
        Director::getInstance()->replaceScene(TransitionFade::create(0.25f, game));
        });

    level2Item->setPosition(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.52f-50);
    // 返回（方便调试）
    auto backLabel = Label::createWithTTF("Back", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 40);
    auto backItem = MenuItemLabel::create(backLabel, [](Ref*) {
        auto scene = StartScene::createScene();
        Director::getInstance()->replaceScene(TransitionFade::create(0.2f, scene));
        });

    backItem->setPosition(origin.x + visibleSize.width * 0.5f,
        origin.y + visibleSize.height * 0.28f);

    auto menu = Menu::create(level1Item,level2Item, backItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 2);

    return true;
}
