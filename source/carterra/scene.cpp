#include "sme/carterra/scene.h"
#include "sme/carterra/camera.h"
#include "sme/carterra/map.h"
#include "sme/carterra/player.h"
#include "tsuru/debug/debugscene.h"
#include "game/actor/actormgr.h"
#include "sead/heapmgr.h"
#include "log.h"

SEAD_SINGLETON_TASK_IMPL(crt::Scene);

crt::Scene::Scene(const sead::TaskConstructArg& arg)
    : sead::CalculateTask(arg, "CarterraScene")
    , renderer()
    , controllers()
    , camera(nullptr)
    , map(nullptr)
{ }

crt::Scene::~Scene() {
    this->camera->isDeleted = true;
    this->map->isDeleted = true;
}

sead::TaskBase* crt::Scene::construct(const sead::TaskConstructArg& arg) {
    return new (arg.heapArray->heaps[arg.heapArray->primaryIndex], 4) crt::Scene(arg);
}

void crt::Scene::prepare() {
    ActorMgr::createInstance(nullptr)->createHeaps(nullptr);

    $(Actor*)(const ProfileID::__type__ id, u32 settings1 = 0, u32 rotation = 0)(
        ActorBuildInfo buildInfo = { 0 };
        buildInfo.profile = Profile::get(id);
        buildInfo.settings1 = settings1;
        buildInfo.rotation = rotation;
        return ActorMgr::instance()->create(buildInfo);
    ) spawnActor;

    this->map = static_cast<crt::Map*>(spawnActor(ProfileID::CarterraMap, 4, (u32)sead::HeapMgr::instance()->getCurrentHeap()));
    this->player = static_cast<crt::Player*>(spawnActor(ProfileID::CarterraPlayer));
    this->player->position = this->map->getBonePos(this->map->map->nodes[0]->boneName);
    this->camera = static_cast<crt::Camera*>(spawnActor(ProfileID::CarterraCamera));

    this->renderer.init(this->camera);

    this->controllers.init();

    this->adjustHeapAll();
}

void crt::Scene::enter() {
    this->renderer.start();
}

void crt::Scene::calc() {
    ActorMgr::instance()->executeActors();
}
