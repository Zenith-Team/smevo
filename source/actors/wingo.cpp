#include "tsuru/actor/bosswrapper.h"
#include "game/graphics/model/blendingmodel.h"

class Wingo : public BossWrapper<18> {
    SEAD_RTTI_OVERRIDE_IMPL(Wingo, Boss);

public:
    Wingo(const ActorBuildInfo* buildInfo);
    virtual ~Wingo() { }

    u32 onCreate() override;
    u32 onExecute() override;
    u32 onDraw() override;

    void initHitboxCollider() override;
    void initModels() override;
    void updateModel() override;
    void onStompDamage(StageActor*) override;
    void onStompKill(StageActor*) override;

    static const HitboxCollider::Info collisionInfo;

    static void collisionCallback(HitboxCollider* hcSelf, HitboxCollider* hcOther);

    BlendingModel* model;
};

REGISTER_PROFILE(Wingo, ProfileID::Wingo);
PROFILE_RESOURCES(ProfileID::Wingo, Profile::LoadResourcesAt::Course, "star_coin");

const HitboxCollider::Info Wingo::collisionInfo = {
    Vec2f(0.0f, 6.0f), Vec2f(14.0f, 22.0f), HitboxCollider::Shape::Rectangle, 5, 0, 0x824F, 0x20208, 0, &Wingo::collisionCallback
};

Wingo::Wingo(const ActorBuildInfo* buildInfo)
    : BossWrapper<18>(buildInfo)
    , model(nullptr)
{ }

u32 Wingo::onCreate() {
    return BossWrapper<18>::onCreate();
}

u32 Wingo::onExecute() {
    this->states.execute();
    this->updateModelTrampoline();

    return 1;
}

u32 Wingo::onDraw() {
    this->model->draw();

    return 0;
}

void Wingo::initHitboxCollider() {
    this->hitboxCollider.init(this, &Wingo::collisionInfo);
}

void Wingo::initModels() {
    this->model = BlendingModel::create("star_coin", "star_coinA", 0);
}

void Wingo::updateModel() {
    Mtx34 mtx;
    mtx.makeRTIdx(this->rotation, this->position);
    this->model->setScale(2.0f);
    this->model->setMtx(mtx);
    this->model->updateAnimations();
    this->model->updateModel();
}

void Wingo::onStompDamage(StageActor* collidingActor) {
    return BossWrapper<18>::onStompDamage(collidingActor);
}

void Wingo::onStompKill(StageActor* collidingActor) {
    return BossWrapper<18>::onStompKill(collidingActor);
}

void Wingo::collisionCallback(HitboxCollider* hcSelf, HitboxCollider* hcOther) {
    return BossWrapper<18>::collisionCallback(hcSelf, hcOther);
}
