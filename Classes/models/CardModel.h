// src/models/CardModel.h
#pragma once
#include "cocos2d.h"

enum class CardSuit
{
    Spade,
    Heart,
    Club,
    Diamond
};
enum class CardColor
{
    Red,
    Black
};
struct CardModel
{
    int id = -1;
    CardColor color=CardColor::Black;
    int value = 1;              // 1¨C13
    CardSuit suit = CardSuit::Spade;
    cocos2d::Vec2 pos;          // ÊÀ½ç×ø±ê
};
