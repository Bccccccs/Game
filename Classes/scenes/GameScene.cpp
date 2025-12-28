#include "scenes/GameScene.h"
#include "controllers/GameController.h"

USING_NS_CC;

Scene* GameScene::createScene(int levelIndex)
{
    auto scene = GameScene::create();
    scene->setLevelIndex(levelIndex);
    return scene;
}

void GameScene::setLevelIndex(int levelIndex)
{
    _levelIndex = levelIndex;
}

bool GameScene::init()
{
    if (!Scene::init()) return false;

    auto size = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto bg = LayerColor::create(Color4B(20, 120, 60, 255));
    this->addChild(bg, 0);

    // Undo °´Å¥£¨ÓÒÉÏ½Ç£©
    auto undoLabel = Label::createWithTTF("UNDO", "D:/CardGame/MyCppGame/Resources/fonts/ArtierEN-2.ttf", 36);
    auto undoItem = MenuItemLabel::create(undoLabel, [this](Ref*) {
        if (_controller) _controller->undo();
        });
    undoItem->setPosition(origin.x + size.width - 90, origin.y + size.height - 50);

    auto menu = Menu::create(undoItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 10);

    // controller
    _controller = new GameController(this);
    _controller->initLevel1();

    return true;
}

void GameScene::onExit()
{
    if (_controller)
    {
        delete _controller;
        _controller = nullptr;
    }
    Scene::onExit();
}
