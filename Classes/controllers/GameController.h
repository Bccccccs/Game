#pragma once
#include "cocos2d.h"
#include <unordered_map>
#include <vector>
#include <array>

#include "models/GameModel.h"
#include "models/UndoRecord.h"
#include "views/CardView.h"

class GameController
{
public:
    explicit GameController(cocos2d::Scene* scene);

    void initLevel1();

    void onMainPileTapped(int pileIndex);
    void onStockTapped();     // 备用堆点击：仅在无可匹配时才允许抽牌
    void undo();              // 你已有可先保留，后面我再给你对齐新规则

private:
    std::vector<CardModel> generateRandomDeck();
    void dealNewGame();       // 发牌：3摞主牌 + 1张手牌 + 备用堆

    void renderInitial();
    void refreshStockUI();
    void refreshMainTop(int pileIndex);

    bool canTakeFromMain(int pileIndex) const;
    bool hasAnyMatch() const;

    void takeFromMainToHand(int pileIndex);
    void drawFromStockToHand();   // 无可匹配时抽一张进手牌

    void checkWin();
    void showSuccessLayer();

private:
    cocos2d::Scene* _scene = nullptr;

    // 牌面 view 管理
    std::unordered_map<int, CardView*> _views;

    // 位置
    cocos2d::Vec2 _handPos;
    cocos2d::Vec2 _stockPos;
    std::array<cocos2d::Vec2, 3> _mainPos{};

    // 数据：手牌永远 1 张
    CardModel _hand;

    // 主牌堆：三摞，每摞只展示顶牌
    std::array<std::vector<CardModel>, 3> _mainPiles;

    // 备用牌堆：一摞背面
    std::vector<CardModel> _stock;

    // 弃牌堆（不显示，用来保存历史，便于 undo/统计；你不需要也可以删）
    std::vector<CardModel> _discard;

    // UI：备用堆背面+数量
    cocos2d::Sprite* _stockBack = nullptr;
    cocos2d::Label* _stockCountLabel = nullptr;

    // 手牌 view
    CardView* _handView = nullptr;
    // Undo 栈
    std::vector<UndoRecord> _undo;

};
