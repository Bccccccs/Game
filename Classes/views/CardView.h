// src/views/CardView.h
#pragma once
#include "cocos2d.h"
#include "models/CardModel.h"
#include <functional>

class CardView : public cocos2d::Sprite
{
public:
    static CardView* createWithModel(const CardModel& model);

    int getCardId() const { return _cardId; }
    void setCardId(int id) { _cardId = id; }

    using TapCallback = std::function<void(int cardId)>;
    void setOnTap(TapCallback cb) { _onTap = std::move(cb); }

protected:
    void onEnter() override;
    void onExit() override;

private:
    bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

private:
    int _cardId = -1;
    TapCallback _onTap;
    cocos2d::EventListenerTouchOneByOne* _touchListener = nullptr;
};
