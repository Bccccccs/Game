// src/views/CardView.cpp
#include "views/CardView.h"
#include "cocos2d.h"
#include <string>

USING_NS_CC;

static const char* kPlaceholderPng = "card_general.png";
static std::string get_color(CardModel model)
{
    if (model.color == CardColor::Red)
        return "red";
    else
        return "black";

}
static std::string get_suit(CardModel model)
{
    switch (model.suit)
    {
    case CardSuit::Spade:return "spade";
    case CardSuit::Diamond:return "diamond";
    case CardSuit::Club:return "club";
    case CardSuit::Heart:return "heart";
    default:return "unknown";
    }
}
CardView* CardView::createWithModel(const CardModel& model)
{
    bool exists = FileUtils::getInstance()->isFileExist(kPlaceholderPng);
    CCLOG("[CardView] png exists=%d, file=%s", exists ? 1 : 0, kPlaceholderPng);

    auto card = new (std::nothrow) CardView();
    if (!card)
    {
        CCLOG("[CardView] new CardView failed");
        return nullptr;
    }

    if (!exists)
    {
        std::string full = FileUtils::getInstance()->fullPathForFilename(kPlaceholderPng);
        CCLOG("[CardView] file not found, fullPath=%s", full.c_str());
        CC_SAFE_DELETE(card);
        return nullptr;
    }
    if (card->initWithFile(kPlaceholderPng))
    {
        card->autorelease();
        card->setPosition(model.pos);
        card->setCardId(model.id);

        // === 数字图片 ===
        // 资源路径：resource/fonts/numbers/big_black_1.png 之类
        std::string suitnumPath = "D:/CardGame/MyCppGame/Resources/suits/" + get_suit(model)+".png";
        //CCLOG(get_suit(model));
        std::string smallnumPath = "D:/CardGame/MyCppGame/Resources/number/small_" + get_color(model) + "_" + std::to_string(model.value) + ".png";
        std::string bignumPath = "D:/CardGame/MyCppGame/Resources/number/big_"+get_color(model)+"_" + std::to_string(model.value) + ".png";
        bool numExists = FileUtils::getInstance()->isFileExist(suitnumPath);
        CCLOG("[CardView] number exists=%d path=%s",
            numExists ? 1 : 0,
            suitnumPath.c_str());
        if (FileUtils::getInstance()->isFileExist(bignumPath))
        {
            auto suitnumSprite = Sprite::create(suitnumPath);
            auto numSprite = Sprite::create(bignumPath);
            auto smallnumSprite = Sprite::create(smallnumPath);
            if (numSprite)
            {
                auto cs = card->getContentSize();
                numSprite->setPosition(Vec2(cs.width * 0.5f, cs.height * 0.5f));
                card->addChild(numSprite, 10); // 保证在牌面上层
            }
            if (smallnumSprite)
            {
                auto cs = card->getContentSize();
                smallnumSprite->setPosition(Vec2(cs.width * 0.4f, cs.height * 0.9f));
                card->addChild(smallnumSprite, 11); // 保证在牌面上层
            }
            if (suitnumSprite)
            {
                auto cs = card->getContentSize();
                suitnumSprite->setPosition(Vec2(cs.width * 0.15f, cs.height * 0.9f));
                card->addChild(suitnumSprite, 12); // 保证在牌面上层
            }

        }
        else
        {
            CCLOG("[CardView] number image not found: %s", bignumPath.c_str());
        }

        // 如果你的 card_general.png 尺寸太大/太小，可在这里统一缩放
        // card->setScale(0.5f);

        return card;
    }


    std::string full = FileUtils::getInstance()->fullPathForFilename(kPlaceholderPng);
    CCLOG("[CardView] initWithFile failed, fullPath=%s", full.c_str());

    CC_SAFE_DELETE(card);
    return nullptr;
}

void CardView::onEnter()
{
    Sprite::onEnter();

    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);
    _touchListener->onTouchBegan = CC_CALLBACK_2(CardView::onTouchBegan, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardView::onExit()
{
    if (_touchListener)
    {
        _eventDispatcher->removeEventListener(_touchListener);
        _touchListener = nullptr;
    }
    Sprite::onExit();
}

bool CardView::onTouchBegan(Touch* touch, Event* event)
{
    // 将触摸点转到本节点坐标
    Vec2 local = this->convertToNodeSpace(touch->getLocation());

    // 以左下角为 (0,0) 的内容矩形做命中判断
    Rect rect(0, 0, this->getContentSize().width, this->getContentSize().height);
    if (!rect.containsPoint(local))
        return false;

    CCLOG("[CardView] tapped cardId=%d", _cardId);

    if (_onTap)
        _onTap(_cardId);

    return true;
}
