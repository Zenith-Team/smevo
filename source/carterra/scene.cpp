#include "tsuru/carterra/scene.h"
#include "tsuru/carterra/camera.h"
#include "tsuru/carterra/map.h"
#include "tsuru/debug/debugscene.h"
#include "game/actor/actormgr.h"
#include "game/task/taskmgr.h"
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

    this->camera = static_cast<crt::Camera*>(spawnActor(ProfileID::CarterraCamera));
    this->map = static_cast<crt::Map*>(spawnActor(ProfileID::CarterraMap, 4, (u32)sead::HeapMgr::instance()->getCurrentHeap()));

    this->renderer.init(this->camera);

    this->controllers.init();

    this->adjustHeapAll();
}

void crt::Scene::enter() {
    this->renderer.start();
}

void crt::Scene::calc() {
    ActorMgr::instance()->executeActors();

    if (this->controllers.buttonDown(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.x -= 1;
    }

    if (this->controllers.buttonUp(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.x += 1;
    }

    if (this->controllers.buttonLeft(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.z -= 1;
    }

    if (this->controllers.buttonRight(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.z += 1;
    }

    if (this->controllers.buttonPlus(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.y += 1;
    }

    if (this->controllers.buttonMinus(InputControllers::ControllerID::Gamepad)) {
        this->camera->position.y -= 1;
    }

    if (this->controllers.buttonA(InputControllers::ControllerID::Gamepad)) {
        sead::TaskClassID debugScene;
        debugScene.type = sead::TaskClassID::Type_Factory;
        debugScene.id.factory = &DebugScene::construct;
        
        TaskMgr::instance()->changeTask(this, debugScene);
    }
}
