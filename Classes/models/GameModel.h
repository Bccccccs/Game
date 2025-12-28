
#pragma once
#include <vector>
#include "models/CardModel.h"

struct GameModel
{
    std::vector<CardModel> tableCards;
    CardModel currentCard;

    // 新增：备用牌堆（栈顶放在 back）
    std::vector<CardModel> stackCards;
};
