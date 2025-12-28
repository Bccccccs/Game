#include "controllers/GameController.h"
#include "scenes/GameScene.h"
#include "scenes/LevelSelectScene.h"

#include <algorithm>
#include <random>
#include <ctime>
#include <cmath>

USING_NS_CC;

static const char* kCardBackPng = "card_general.png";
static const int kSuccessLayerTag = 999001;

static CardColor colorFromSuit(CardSuit suit)
{
    if (suit == CardSuit::Heart || suit == CardSuit::Diamond) return CardColor::Red;
    return CardColor::Black;
}

GameController::GameController(Scene* scene)
    : _scene(scene)
{
}

std::vector<CardModel> GameController::generateRandomDeck()
{
    std::vector<CardModel> deck;
    deck.reserve(52);

    int idCounter = 1;
    for (int v = 1; v <= 13; ++v)
    {
        for (CardSuit s : { CardSuit::Spade, CardSuit::Heart, CardSuit::Club, CardSuit::Diamond })
        {
            CardModel c;
            c.id = idCounter++;
            c.suit = s;
            c.color = colorFromSuit(s);
            c.value = v;
            deck.push_back(c);
        }
    }

    std::default_random_engine rng((unsigned)time(nullptr));
    std::shuffle(deck.begin(), deck.end(), rng);
    return deck;
}

void GameController::initLevel1()
{
    // 如果你多次进入关卡，建议先清理旧 view（这里做最基本清理）
    for (auto& kv : _views)
        if (kv.second) kv.second->removeFromParent();
    _views.clear();

    if (_handView) { _handView->removeFromParent(); _handView = nullptr; }

    if (_stockBack) { _stockBack->removeFromParent(); _stockBack = nullptr; }
    if (_stockCountLabel) { _stockCountLabel->removeFromParent(); _stockCountLabel = nullptr; }

    // 清掉成功层
    if (auto scene = Director::getInstance()->getRunningScene())
        if (auto win = scene->getChildByTag(kSuccessLayerTag))
            win->removeFromParent();

    dealNewGame();
    renderInitial();
}

void GameController::dealNewGame()
{
    auto size = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    // 布局：主牌三摞在上方，手牌居中偏下，备用堆右下
    _mainPos[0] = origin + Vec2(size.width * 0.30f, size.height * 0.62f);
    _mainPos[1] = origin + Vec2(size.width * 0.50f, size.height * 0.62f);
    _mainPos[2] = origin + Vec2(size.width * 0.70f, size.height * 0.62f);

    _handPos = origin + Vec2(size.width * 0.50f, size.height * 0.22f);
    _stockPos = origin + Vec2(size.width * 0.85f, size.height * 0.22f);

    for (auto& pile : _mainPiles) pile.clear();
    _stock.clear();
    _discard.clear();

    auto deck = generateRandomDeck();

    // 主牌三摞：每摞发固定数量（你可调整，越大越像正式关卡）
    const int mainEach = 3; // 每摞10张，总共30张做主牌
    for (int i = 0; i < 3; ++i)
    {
        _mainPiles[i].reserve(mainEach);
        for (int k = 0; k < mainEach; ++k)
        {
            CardModel c = deck.back(); deck.pop_back();
            c.pos = _mainPos[i];
            _mainPiles[i].push_back(c);
        }
    }

    // 手牌 1 张（等待匹配区永远只显示这一张）
    _hand = deck.back(); deck.pop_back();
    _hand.pos = _handPos;

    // 剩下的全部进备用堆（背面一摞，不摊开）
    for (auto& c : deck) c.pos = _stockPos;
    _stock = std::move(deck);
}

void GameController::renderInitial()
{
    // 1) 渲染主牌三摞：每摞只显示顶牌
    for (int i = 0; i < 3; ++i)
        refreshMainTop(i);

    // 2) 渲染手牌：只显示 1 张
    _handView = CardView::createWithModel(_hand);
    if (_handView)
    {
        _handView->setOnTap([this](int) {
            CCLOG("[GameController] hand tapped (no action)");
            });
        _scene->addChild(_handView, 5);
        _views[_hand.id] = _handView;
    }

    // 3) 渲染备用堆：只显示背面一摞 + 数量，点击抽牌
    _stockBack = Sprite::create(kCardBackPng);
    if (_stockBack)
    {
        _stockBack->setPosition(_stockPos);
        _scene->addChild(_stockBack, 1);

        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [this](Touch* t, Event*) {
            if (!_stockBack || !_stockBack->isVisible()) return false;
            Vec2 local = _stockBack->convertToNodeSpace(t->getLocation());
            Rect r(0, 0, _stockBack->getContentSize().width, _stockBack->getContentSize().height);
            if (!r.containsPoint(local)) return false;
            this->onStockTapped();
            return true;
            };
        _scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _stockBack);
    }

    _stockCountLabel = Label::createWithSystemFont("", "Arial", 28);
    if (_stockCountLabel)
    {
        _stockCountLabel->setAnchorPoint(Vec2(0.5f, 1.0f));
        _stockCountLabel->setPosition(_stockPos + Vec2(0, -90));
        _scene->addChild(_stockCountLabel, 6);
    }

    refreshStockUI();
}

void GameController::refreshStockUI()
{
    if (_stockCountLabel)
        _stockCountLabel->setString("Stock: " + std::to_string((int)_stock.size()));

    if (_stockBack)
        _stockBack->setVisible(!_stock.empty());
}

void GameController::refreshMainTop(int pileIndex)
{
    // 先移除旧顶牌 view（如果存在）
    // 我们用一个约定：主牌顶牌 view 仍然存在 _views 中，按牌 id 找并删即可；
    // 简化做法：刷新时，把该 pile 的所有可能显示的顶牌（其实只有一个）删掉再建新的。

    // 找到当前 pile 顶牌的 id（若为空则不显示）
    if (_mainPiles[pileIndex].empty())
        return;

    // 顶牌
    auto& top = _mainPiles[pileIndex].back();
    top.pos = _mainPos[pileIndex];

    // 如果该 id 已经有 view，先确保位置对（一般不会重复创建）
    auto it = _views.find(top.id);
    if (it != _views.end() && it->second)
    {
        it->second->setPosition(_mainPos[pileIndex]);
        return;
    }

    auto v = CardView::createWithModel(top);
    if (!v) return;

    v->setOnTap([this, pileIndex](int) {
        this->onMainPileTapped(pileIndex);
        });

    _scene->addChild(v, 3);
    _views[top.id] = v;
}

bool GameController::canTakeFromMain(int pileIndex) const
{
    if (_mainPiles[pileIndex].empty()) return false;

    int v = _mainPiles[pileIndex].back().value;
    int h = _hand.value;

    // 常规相邻
    if (std::abs(v - h) == 1)
        return true;

    // A(1) <-> K(13) 特殊规则
    if ((v == 1 && h == 13) || (v == 13 && h == 1))
        return true;

    return false;
}

bool GameController::hasAnyMatch() const
{
    return canTakeFromMain(0) || canTakeFromMain(1) || canTakeFromMain(2);
}

void GameController::onMainPileTapped(int pileIndex)
{
    if (!canTakeFromMain(pileIndex))
    {
        CCLOG("[GameController] reject: main pile %d top not match hand", pileIndex);
        return;
    }
    takeFromMainToHand(pileIndex);
}

void GameController::takeFromMainToHand(int pileIndex)
{
    if (_mainPiles[pileIndex].empty()) return;

    // 顶牌 -> 手牌
    // 顶牌 -> 手牌
    CardModel picked = _mainPiles[pileIndex].back();

    // 记录 undo：从哪一摞拿的、拿的哪张、之前手牌是什么、从哪儿来
    {
        UndoRecord rec;
        rec.type = UndoType::TakeFromTable;
        rec.movedCard = picked;          // 本次变成手牌的牌
        rec.prevCurrent = _hand;         // 操作前的手牌
        rec.movedFromPos = _mainPos[pileIndex]; // 主牌摞顶牌位置
        rec.tableIndex = pileIndex;      // 借用 tableIndex 存 pileIndex(0/1/2)
        _undo.push_back(rec);
    }

    _mainPiles[pileIndex].pop_back();

    // 当前手牌进弃牌堆（不显示）
    _discard.push_back(_hand);

    // 动画：把主牌顶牌 view 移动到 hand 位置，然后把它变成新的 handView
    CardView* moving = nullptr;
    auto it = _views.find(picked.id);
    if (it != _views.end()) moving = it->second;

    // 移除旧手牌 view
    if (_handView)
    {
        _handView->removeFromParent();
        _views.erase(_hand.id);
        _handView = nullptr;
    }

    // 更新 hand 数据
    picked.pos = _handPos;
    _hand = picked;

    if (moving)
    {
        moving->stopAllActions();
        auto move = MoveTo::create(0.22f, _handPos);

        auto finish = CallFunc::create([this, pileIndex, moving]() {
            // moving 现在就是新的手牌 view
            _handView = moving;
            _handView->setLocalZOrder(5);
            _handView->setOnTap([this](int) {
                CCLOG("[GameController] hand tapped (no action)");
                });

            // 该主牌摞露出下一张：刷新顶牌显示（只显示一张）
            // 注意：要把上一张顶牌 view 从 _views 中移掉吗？
            // picked 的 view 已经成了手牌，所以 pile 的旧顶牌不需要删。
            // 只要创建新的顶牌即可。
            this->refreshMainTop(pileIndex);

            this->checkWin();
            });

        moving->runAction(Sequence::create(move, finish, nullptr));
    }
    else
    {
        // 极端情况：找不到 view，就直接重建手牌 view
        _handView = CardView::createWithModel(_hand);
        if (_handView)
        {
            _handView->setOnTap([this](int) { CCLOG("[GameController] hand tapped"); });
            _scene->addChild(_handView, 5);
            _views[_hand.id] = _handView;
        }

        refreshMainTop(pileIndex);
        checkWin();
    }
}

void GameController::onStockTapped()
{
    // 只有在“完全无可匹配”时才允许抽牌（符合你的描述）
    if (hasAnyMatch())
    {
        CCLOG("[GameController] stock tap ignored: still has match");
        return;
    }
    drawFromStockToHand();
}

void GameController::drawFromStockToHand()
{
    if (_stock.empty())
    {
        CCLOG("[GameController] stock empty");
        return;
    }

    // 抽一张进手牌；旧手牌进入弃牌堆（不显示）
    _discard.push_back(_hand);

    CardModel drawn = _stock.back();

    // 记录 undo：从备用堆抽牌、之前手牌是什么
    {
        UndoRecord rec;
        rec.type = UndoType::DrawFromStack;
        rec.movedCard = drawn;
        rec.prevCurrent = _hand;
        rec.movedFromPos = _stockPos;
        rec.tableIndex = -1;
        _undo.push_back(rec);
    }

    _stock.pop_back();
    refreshStockUI();


    // 用背面飞到手牌位置，再替换成正面（视觉更像正式纸牌）
    auto flyBack = Sprite::create(kCardBackPng);
    if (!flyBack) return;
    flyBack->setPosition(_stockPos);
    _scene->addChild(flyBack, 20);

    // 删旧手牌 view
    if (_handView)
    {
        _handView->removeFromParent();
        _views.erase(_hand.id);
        _handView = nullptr;
    }

    auto move = MoveTo::create(0.22f, _handPos);
    auto finish = CallFunc::create([this, flyBack, drawn]() mutable {
        if (flyBack) flyBack->removeFromParent();

        drawn.pos = _handPos;
        _hand = drawn;

        _handView = CardView::createWithModel(_hand);
        if (_handView)
        {
            _handView->setOnTap([this](int) { CCLOG("[GameController] hand tapped"); });
            _scene->addChild(_handView, 5);
            _views[_hand.id] = _handView;
        }

        // 抽牌不直接胜利检查；胜利由主牌清空决定
        });

    flyBack->runAction(Sequence::create(move, finish, nullptr));
}

void GameController::checkWin()
{
    // 胜利条件：三摞主牌都清空（手牌/备用堆不用清空）
    bool win = _mainPiles[0].empty() && _mainPiles[1].empty() && _mainPiles[2].empty();
    if (win) showSuccessLayer();
}

void GameController::showSuccessLayer()
{
    auto scene = Director::getInstance()->getRunningScene();
    if (!scene) return;
    if (scene->getChildByTag(kSuccessLayerTag)) return;

    auto size = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto mask = LayerColor::create(Color4B(0, 0, 0, 180));
    mask->setTag(kSuccessLayerTag);
    scene->addChild(mask, 999);

    auto swallow = EventListenerTouchOneByOne::create();
    swallow->setSwallowTouches(true);
    swallow->onTouchBegan = [](Touch*, Event*) { return true; };
    mask->getEventDispatcher()->addEventListenerWithSceneGraphPriority(swallow, mask);

    auto label = Label::createWithSystemFont("SUCCESS", "Arial", 72);
    label->setPosition(origin + Vec2(size.width * 0.5f, size.height * 0.62f));
    mask->addChild(label, 1);

    auto restartLabel = Label::createWithSystemFont("Restart", "Arial", 40);
    auto restartItem = MenuItemLabel::create(restartLabel, [](Ref*) {
        Director::getInstance()->replaceScene(
            TransitionFade::create(0.3f, GameScene::createScene(1))
        );
        });

    auto backLabel = Label::createWithSystemFont("Back", "Arial", 40);
    auto backItem = MenuItemLabel::create(backLabel, [](Ref*) {
        Director::getInstance()->replaceScene(
            TransitionFade::create(0.3f, LevelSelectScene::createScene())
        );
        });

    auto menu = Menu::create(restartItem, backItem, nullptr);
    menu->alignItemsVerticallyWithPadding(18);
    menu->setPosition(origin + Vec2(size.width * 0.5f, size.height * 0.40f));
    mask->addChild(menu, 2);
}
void GameController::undo()
{
    if (_undo.empty())
    {
        CCLOG("[GameController] undo: empty");
        return;
    }

    // 若胜利层存在，撤销时先移除
    if (auto scene = Director::getInstance()->getRunningScene())
    {
        if (auto win = scene->getChildByTag(kSuccessLayerTag))
            win->removeFromParent();
    }

    UndoRecord rec = _undo.back();
    _undo.pop_back();

    // 当前手牌 view 必须存在
    if (!_handView)
    {
        CCLOG("[GameController] undo failed: handView null");
        return;
    }

    // 当前手牌（需要退回去的那张）应该就是 rec.movedCard
    // 但为了稳妥，我们按当前 _hand 来处理
    CardModel curHand = _hand;

    // 先把弃牌堆回退一格（因为每次操作都会 push_old_hand）
    if (!_discard.empty())
        _discard.pop_back();

    // 先停止手牌动作
    _handView->stopAllActions();

    // 目标位置：退回主牌摞 or 备用堆
    Vec2 backPos = rec.movedFromPos;

    auto moveBack = MoveTo::create(0.20f, backPos);

    auto finish = CallFunc::create([this, rec, curHand]() mutable {

        // 1) 移除当前手牌 view（curHand）
        if (_handView)
        {
            _handView->removeFromParent();
            _handView = nullptr;
        }
        _views.erase(curHand.id);

        // 2) 恢复 prevCurrent 为新的手牌，并创建 view
        _hand = rec.prevCurrent;
        _hand.pos = _handPos;

        _handView = CardView::createWithModel(_hand);
        if (_handView)
        {
            _handView->setOnTap([this](int) {
                CCLOG("[GameController] hand tapped (no action)");
                });
            _scene->addChild(_handView, 5);
            _views[_hand.id] = _handView;
        }

        // 3) 把 curHand 退回原来源
        if (rec.type == UndoType::DrawFromStack)
        {
            // 退回备用堆
            CardModel back = rec.movedCard;
            back.pos = _stockPos;
            _stock.push_back(back);
            refreshStockUI();
        }
        else if (rec.type == UndoType::TakeFromTable)
        {
            // 退回主牌摞（tableIndex 里存的是 pileIndex）
            int pileIndex = rec.tableIndex;
            if (pileIndex < 0 || pileIndex > 2)
            {
                CCLOG("[GameController] undo failed: bad pileIndex=%d", pileIndex);
                return;
            }

            // 重要：撤销时，要把“撤销后露出来的那张顶牌 view”隐藏回去
            // 因为我们要把 movedCard 放回去成为顶牌
            if (!_mainPiles[pileIndex].empty())
            {
                int revealedId = _mainPiles[pileIndex].back().id;
                auto it = _views.find(revealedId);
                if (it != _views.end() && it->second)
                {
                    it->second->removeFromParent();
                }
                _views.erase(revealedId);
            }

            // 把 movedCard 放回该摞顶部
            CardModel back = rec.movedCard;
            back.pos = _mainPos[pileIndex];
            _mainPiles[pileIndex].push_back(back);

            // 为这张顶牌创建 view（可点击、可匹配）
            auto v = CardView::createWithModel(_mainPiles[pileIndex].back());
            if (v)
            {
                v->setOnTap([this, pileIndex](int) {
                    this->onMainPileTapped(pileIndex);
                    });
                _scene->addChild(v, 3);
                _views[back.id] = v;
            }
        }
        });

    _handView->runAction(Sequence::create(moveBack, finish, nullptr));
}
