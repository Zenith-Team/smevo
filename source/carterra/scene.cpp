#include "sme/carterra/scene.h"
#include "sme/carterra/camera.h"
#include "sme/carterra/map.h"
#include "sme/carterra/player.h"
#include "game/actor/actormgr.h"
#include "sead/heapmgr.h"
#include "tsuru/layoutrenderer.h"
#include "log.h"

SEAD_SINGLETON_TASK_IMPL(crt::Scene);

namespace crt {
    CREATE_STATE(Scene, Active);
    CREATE_STATE(Scene, Menu);
}

crt::Scene::Scene(const sead::TaskConstructArg& arg)
    : sead::CalculateTask(arg, "CarterraScene")
    , renderer()
    , camera(nullptr)
    , player(nullptr)
    , map(nullptr)
    , controllers()
    , uiLayout()
    , states(this)
{ }

crt::Scene::~Scene() {
    this->camera->isDeleted = true;
    this->map->isDeleted = true;
}

sead::TaskBase* crt::Scene::construct(const sead::TaskConstructArg& arg) {
    return new (arg.heapArray->heaps[arg.heapArray->primaryIndex], 4) crt::Scene(arg);
}

void crt::Scene::prepare() {
    //* Actors

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

    //* Rendering

    this->renderer.init(this->camera);

    LightMapMgr::instance()->setCSLightMaps();

    //* Input

    this->controllers.init();

    //* UI

    this->uiLayout.init();
    this->uiLayout.getArchive("CourseSelect");
    this->uiLayout.loadBFLYT("CourseSelectScene.bflyt");

    // Disable non-default panes
    this->uiLayout.getPane("W_Frame_00")->scale = 0;
    this->uiLayout.getPane("Pa_Return_00")->scale = 0;
    this->uiLayout.getPane("Pa_Comment_00")->scale = 0;
    this->uiLayout.getPane("Pa_ZoomDown_00")->scale = 0;
    this->uiLayout.getPane("Pa_ZoomUp_00")->scale = 0;

    this->adjustHeapAll();
}

void crt::Scene::enter() {
    this->renderer.start();

    this->states.changeState(&crt::Scene::StateID_Active);
}

void crt::Scene::calc() {
    this->states.execute();
}

/** STATE: Active */

void crt::Scene::beginState_Active() { }

void crt::Scene::executeState_Active() {
    ActorMgr::instance()->executeActors();
    
    this->uiLayout.update(0xE);
    LayoutRenderer::instance()->addLayout(this->uiLayout);

    if (this->controllers.buttonPlus(InputControllers::ControllerID::Gamepad)) {
        this->states.changeState(&crt::Scene::StateID_Menu);
    }
}

void crt::Scene::endState_Active() { }

/** STATE: Menu */

void crt::Scene::beginState_Menu() { }

void crt::Scene::executeState_Menu() {
    if (this->controllers.buttonA(InputControllers::ControllerID::Gamepad)) {
        this->states.changeState(&crt::Scene::StateID_Active);
    }
}

void crt::Scene::endState_Menu() { }
