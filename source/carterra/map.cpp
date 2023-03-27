#include "tsuru/carterra/map.h"
#include "tsuru/carterra/scene.h"
#include "dynlibs/os/functions.h"
#include "game/resource/util.h"
#include "sead/heapmgr.h"
#include "log.h"

namespace crt {
    REGISTER_PROFILE(Map, ProfileID::CarterraMap);
}

Actor* crt::Map::build(const ActorBuildInfo* buildInfo) {
    return new Map(buildInfo);
}

crt::Map::Map(const ActorBuildInfo* buildInfo)
    : crt::MapActor(buildInfo)
    , model(nullptr)
    , bones(nullptr)
    , sceneHeap((sead::Heap*)buildInfo->rotation) // Reuse the rotation field to pass the scene heap
{
    sead::HeapMgr::instance()->setCurrentHeap(this->sceneHeap);

    char name[32] = {0};
    char path[64] = {0};
    __os_snprintf(name, 32, "CS_W%d", this->settings1);
    __os_snprintf(path, 64, "course_res_pack/%s.szs", name);

    loadResource(name, path);
}

u32 crt::Map::onCreate() {
    sead::HeapMgr::instance()->setCurrentHeap(this->sceneHeap);

    char name[32] = {0};
    char terrainModelName[32] = {0};

    __os_snprintf(name, 32, "CS_W%d", this->settings1);
    __os_snprintf(terrainModelName, 32, "CS_W%d_World", this->settings1);

    this->model = ModelWrapper::create(name, terrainModelName);
    this->bones = ModelWrapper::create(name, name);

    Mtx34 mtx;
    mtx.makeRTIdx(0, this->position);

    this->bones->setMtx(mtx);
    this->bones->setScale(0.1);
    this->bones->updateModel();

    this->model->setMtx(mtx);
    this->model->setScale(0.1);
    this->model->updateModel();

    return 1;
}

u32 crt::Map::onDraw() {
    this->model->draw();
    
    return 1;
}

u32 crt::Map::onDelete() {
    delete this->model;
    delete this->bones;
    
    return 1;
}

Vec3f crt::Map::getBonePos(const sead::SafeString& name) {
    Mtx34 mtx;
    this->bones->model->getBoneWorldMatrix(this->bones->model->searchBoneIndex(name), &mtx);

    return Vec3f(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
}
