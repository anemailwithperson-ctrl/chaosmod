#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cmath>
#include <cstdlib>

using namespace geode::prelude;

static ccColor3B hueToRGB(float h) {
    h = std::fmod(h, 360.f);
    if (h < 0.f) h += 360.f;
    float x = 1.f - std::fabs(std::fmod(h / 60.f, 2.f) - 1.f);
    float r = 0, g = 0, b = 0;
    if (h < 60)       { r = 1; g = x; }
    else if (h < 120) { r = x; g = 1; }
    else if (h < 180) { g = 1; b = x; }
    else if (h < 240) { g = x; b = 1; }
    else if (h < 300) { r = x; b = 1; }
    else              { r = 1; b = x; }
    return { (GLubyte)(r * 255), (GLubyte)(g * 255), (GLubyte)(b * 255) };
}

class $modify(ChaosMenu, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (auto title = this->getChildByID("main-title")) {
            // pulsing
            title->runAction(CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(CCScaleTo::create(0.4f, 1.15f)),
                CCEaseSineInOut::create(CCScaleTo::create(0.4f, 0.9f)),
                nullptr)));
            // wobbling
            title->runAction(CCRepeatForever::create(CCSequence::create(
                CCEaseSineInOut::create(CCRotateTo::create(0.6f, 4.f)),
                CCEaseSineInOut::create(CCRotateTo::create(0.6f, -4.f)),
                nullptr)));
            // rainbow tint
            title->runAction(CCRepeatForever::create(CCSequence::create(
                CCTintTo::create(0.5f, 255, 60, 60),
                CCTintTo::create(0.5f, 255, 200, 40),
                CCTintTo::create(0.5f, 60, 255, 60),
                CCTintTo::create(0.5f, 40, 220, 255),
                CCTintTo::create(0.5f, 120, 60, 255),
                CCTintTo::create(0.5f, 255, 60, 200),
                nullptr)));
        }

        auto size = CCDirector::get()->getWinSize();
        auto label = CCLabelBMFont::create("CHAOS MODE ACTIVATED", "bigFont.fnt");
        label->setScale(0.5f);
        label->setPosition({ size.width / 2, size.height - 15 });
        label->runAction(CCRepeatForever::create(CCSequence::create(
            CCTintTo::create(0.3f, 255, 0, 0),
            CCTintTo::create(0.3f, 255, 255, 0),
            CCTintTo::create(0.3f, 0, 255, 255),
            nullptr)));
        this->addChild(label);
        return true;
    }
};

static const char* ROASTS[] = {
    "Skill issue.", "Again?!", "Not even close.", "The spike was RIGHT there.",
    "Bro.", "Lag. Definitely lag.", "You were so close... jk", "Touch grass?",
    "Maybe try practice mode.", "That one's on the game."
};

class $modify(ChaosPlay, PlayLayer) {
    struct Fields {
        float hue = 0.f;
        int deaths = 0;
        CCLabelBMFont* label = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto size = CCDirector::get()->getWinSize();
        m_fields->label = CCLabelBMFont::create("Deaths: 0", "bigFont.fnt");
        m_fields->label->setAnchorPoint({ 0, 1 });
        m_fields->label->setScale(0.5f);
        m_fields->label->setPosition({ 10, size.height - 10 });
        m_fields->label->setZOrder(100);
        this->addChild(m_fields->label);
        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        m_fields->hue += dt * 180.f;
        auto c1 = hueToRGB(m_fields->hue);
        auto c2 = hueToRGB(m_fields->hue + 180.f);

        if (m_player1) {
            m_player1->setColor(c1);
            m_player1->setSecondColor(c2);
        }
        if (m_player2) {
            m_player2->setColor(c2);
            m_player2->setSecondColor(c1);
        }
        if (m_background) {
            m_background->setColor(hueToRGB(m_fields->hue * 0.25f));
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* obj) {
        PlayLayer::destroyPlayer(player, obj);
        if (obj == m_anticheatSpike) return;

        m_fields->deaths++;
        if (m_fields->label) {
            const char* roast = ROASTS[std::rand() % (sizeof(ROASTS) / sizeof(ROASTS[0]))];
            m_fields->label->setString(
                fmt::format("Deaths: {} - {}", m_fields->deaths, roast).c_str());
            m_fields->label->stopAllActions();
            m_fields->label->setScale(0.8f);
            m_fields->label->runAction(CCEaseBounceOut::create(CCScaleTo::create(0.5f, 0.5f)));
        }
    }
};
