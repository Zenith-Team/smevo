#include "tsuru/actor/bosswrapper.h"
#include "game/graphics/model/blendingmodel.h"
#include "math/easing.h"
#include "math/bezier.h"
#include "sead/random.h"

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
    Vec3f centerPoint;
    QuadraticBezier bezier;
    f32 t;
    Easing easer;
    u8 spawnStage : 4;
    bool swoopStage : 1;
    bool initial : 1;
    u16 timer;
    f32 baseline;

    DECLARE_STATE(Wingo, Spawn);
    DECLARE_STATE(Wingo, Chase);
    DECLARE_STATE(Wingo, Swoop);
    DECLARE_STATE(Wingo, Blow);
    DECLARE_STATE(Wingo, PrepareChase);
};

CREATE_STATE(Wingo, Spawn);
CREATE_STATE(Wingo, Chase);
CREATE_STATE(Wingo, Swoop);
CREATE_STATE(Wingo, Blow);
CREATE_STATE(Wingo, PrepareChase);

REGISTER_PROFILE(Wingo, ProfileID::Wingo);
PROFILE_RESOURCES(ProfileID::Wingo, Profile::LoadResourcesAt::Course, "birdbird");

const HitboxCollider::Info Wingo::collisionInfo = {
    Vec2f(0.0f, 46.0f), Vec2f(30.0f, 48.0f), HitboxCollider::Shape::Rectangle, 5, 0, 0x824F, 0x20208, 0, &Wingo::collisionCallback
};

Wingo::Wingo(const ActorBuildInfo* buildInfo)
    : BossWrapper<18>(buildInfo)
    , model(nullptr)
    , centerPoint(buildInfo->position)
    , bezier()
    , t(0.0f)
    , easer()
    , spawnStage(0)
    , swoopStage(0)
    , initial(true)
    , timer(0)
    , baseline(buildInfo->position.y)
{ }

u32 Wingo::onCreate() {
    this->doStateChange(&Wingo::StateID_Spawn);

    return BossWrapper<18>::onCreate();
}

u32 Wingo::onExecute() {
    this->states.execute();
    this->updateModelTrampoline();

    sead::Mathu::chase(&this->rotation.y, Direction::directionToRotationList[this->directionToPlayerH(this->position)], fixDeg(1.5f));

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
    this->model = BlendingModel::create("birdbird", "birdbird", 2);
}

void Wingo::updateModel() {
    Mtx34 mtx;
    mtx.makeRTIdx(this->rotation, this->position);
    this->model->setScale(0.07f);
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

/** STATE: Spawn */

void Wingo::beginState_Spawn() {
    this->easer.set(Easing::quadInOut, 0.0f, 1.0f, 0.006f);
    this->bezier.set(this->centerPoint + Vec3f(-14*16, 14*16, 0), this->centerPoint + Vec3f(-14*16, 0, 0), this->centerPoint + Vec3f(14*18, 0, 0));
}

void Wingo::executeState_Spawn() {
    bool done = this->easer.ease(this->t);
    this->bezier.execute(&this->position, this->t);

    if (done) {
        this->spawnStage++;
        this->easer.set(Easing::quadInOut, 0.0f, 1.0f, 0.006f);
    
        switch (this->spawnStage) {
            case 1: this->bezier.set(this->centerPoint + Vec3f(14*16, 14*16, 0), this->centerPoint + Vec3f(14*16, 0, 0), this->centerPoint + Vec3f(-14*18, 0, 0)); break;
            case 2: this->doStateChange(&Wingo::StateID_PrepareChase); break;
        }
    }
}

void Wingo::endState_Spawn() { }

/** STATE: Chase */

void Wingo::beginState_Chase() {
    this->speed.x = 0.0f;
    this->timer = 0;
}

void Wingo::executeState_Chase() {    
    if (this->speed.x <= 2.8f) {
        this->speed.x += 0.08f;
    }

    this->position.x += this->speed.x;

    this->timer++;

    if (this->distanceToPlayer().x < 0.0f) {
        this->doStateChange(&Wingo::StateID_Swoop);
    }

    if (this->timer > 60*3 && sead::randU32(100) == 0) {
        this->doStateChange(&Wingo::StateID_Swoop);
    }

    if (this->timer > 60*5) {
        this->doStateChange(&Wingo::StateID_Swoop);
    }
}

void Wingo::endState_Chase() { }

/** STATE: Swoop */

void Wingo::beginState_Swoop() {
    this->swoopStage = 0;
    this->speed.y = 0.0f;
    this->centerPoint = this->position + Vec3f(this->distanceToPlayer());
    this->centerPoint.y = this->baseline;
}

void Wingo::executeState_Swoop() {
    if (this->swoopStage == 0) {
        if (this->speed.y <= 4.0f) {
            this->speed.y += 0.05f;
        } else {
            this->centerPoint = this->position + Vec3f(this->distanceToPlayer());
            this->centerPoint.y = this->baseline;
            this->swoopStage = 1;
            this->easer.set(Easing::quadInOut, 0.0f, 1.0f, 0.006f);
            this->bezier.set(this->centerPoint + Vec3f(-14*16, 14*16, 0), this->centerPoint + Vec3f(0, -14*16, 0), this->centerPoint + Vec3f(14*16, 14*16, 0));
        }

        this->position.y += this->speed.y;
        this->position.x += this->speed.x;
    } else {
        bool done = this->easer.ease(this->t);
        this->bezier.execute(&this->position, this->t);
    
        if (done) {
            this->position = this->centerPoint + Vec3f(-12*16, 0, 0);

            this->doStateChange(&Wingo::StateID_PrepareChase);
        }
    }
}

void Wingo::endState_Swoop() { }

/** STATE: Blow */

void Wingo::beginState_Blow() { }

void Wingo::executeState_Blow() { }

void Wingo::endState_Blow() { }

/** STATE: PrepareChase */

void Wingo::beginState_PrepareChase() {
    this->centerPoint = this->position + Vec3f(this->distanceToPlayer());
    this->centerPoint.y = this->baseline;
    this->bezier.set(this->centerPoint + Vec3f(-17*16, 16*16, 0), this->centerPoint + Vec3f(-16*16, 0, 0), this->centerPoint + Vec3f(-8*16, 0, 0));
    this->easer.set(Easing::quadInOut, 0.0f, 1.0f, 0.006f);

    if (!this->initial) {
        this->bezier.keyframes[2].x = this->centerPoint.x;
    }
}

void Wingo::executeState_PrepareChase() {
    bool done = this->easer.ease(this->t);
    this->bezier.execute(&this->position, this->t);

    if (done) {
        this->doStateChange(&Wingo::StateID_Chase);
    }
}

void Wingo::endState_PrepareChase() {
    if (this->initial) {
        this->finishedSpawning();
        this->initial = false;
    }
}
