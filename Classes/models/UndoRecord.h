#pragma once
#include "models/CardModel.h"

enum class UndoType
{
    TakeFromTable,
    DrawFromStack
};

struct UndoRecord
{
    UndoType type;

    // 本次变成 current 的牌（移动牌）
    CardModel movedCard;

    // 操作前的 current（被替换的牌）
    CardModel prevCurrent;

    // movedCard 移动前所在位置（桌面位置 or 备用牌堆位置）
    cocos2d::Vec2 movedFromPos;

    // 如果是从桌面取走，需要恢复到 tableCards 的原位置（保持顺序）
    int tableIndex = -1;
};
