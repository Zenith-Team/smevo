#include "game/actor/stage/stageactor.h"
#include "game/actor/actoradditionalheap.h"
#include "sead/heapmgr.h"
#include "game/layout/layoutcontainer.h"
#include "game/graphics/model/modelnw.h"
#include "tsuru/layoutrenderer.h"
#include "game/level/levelcamera.h"
#include "log.h"

class NPCSpeechLayout : public LayoutContainer {
public:
    NPCSpeechLayout()
        : LayoutContainer(this->animators, sizeof(this->animators) / sizeof(this->animators[0]))
    { }

    virtual ~NPCSpeechLayout() { }

    LayoutAnimator animators[1];
};

class NPC : public StageActor {
    SEAD_RTTI_OVERRIDE_IMPL(NPC, StageActor);

public:
    NPC(const ActorBuildInfo* buildInfo);
    virtual ~NPC() { }

    u32 onCreate() override;
    u32 onExecute() override;
    u32 onDraw() override;

    ModelWrapper* model;
    NPCSpeechLayout layout;
    ActorAdditionalHeap layoutHeap;
};

REGISTER_PROFILE(NPC, ProfileID::NPC);
PROFILE_RESOURCES(ProfileID::NPC, Profile::LoadResourcesAt::Course, "star_coin");

NPC::NPC(const ActorBuildInfo* buildInfo)
    : StageActor(buildInfo)
    , model(nullptr)
    , layout()
    , layoutHeap()
{ }

u32 NPC::onCreate() {
    this->model = ModelWrapper::create("star_coin", "star_coinA");

    this->layoutHeap.create();

    sead::Heap* currentHeap = sead::HeapMgr::instance()->getCurrentHeap();
    sead::HeapMgr::instance()->setCurrentHeap(this->layoutHeap.heap);

    this->layout.init();
    this->layout.getArchive("Common");
    this->layout.loadBFLYT("BaloonGuide.bflyt");
    this->layout.getTextBoxPane("T_Text_00")->setTextFromMSBT("NPC_Test");

    static const sead::SafeString anims[] = {
        "In"
    };

    //static u32 size = sizeof(anims) / sizeof(anims[0]);
    //this->layout.initAnims(anims, size);
    //this->layout.playAnim(0, "In", false);

    sead::HeapMgr::instance()->setCurrentHeap(currentHeap);

    return this->onExecute();
}

u32 NPC::onExecute() {
    Mtx34 mtx;
    mtx.makeRTIdx(this->rotation, this->position);
    this->model->setMtx(mtx);
    this->model->setScale(this->scale);
    this->model->updateModel();

    const LevelCamera* const cam = LevelCamera::instance();

    Vec3f pos;
    pos.x = (this->position.x - cam->cameraCenterX) / (cam->cameraRight - cam->cameraCenterX);
    pos.y = (this->position.y - cam->cameraCenterY) / (cam->cameraTop - cam->cameraCenterY);
    pos.z = 0.0f;

    mtx.makeT(pos);

    this->layout.update(0xE, &mtx);

    sead::Mathu::chase(&this->rotation.y, Direction::directionToRotationList[this->directionToPlayerH(this->position)], fixDeg(1.75f));

    return 1;
}

u32 NPC::onDraw() {
    this->model->draw();
    LayoutRenderer::instance()->addLayout(this->layout);

    return 1;
}
