#include "tsuru/carterra/camera.h"
#include "tsuru/carterra/scene.h"
#include "tsuru/carterra/map.h"
#include "log.h"

namespace crt {
    REGISTER_PROFILE(Camera, ProfileID::CarterraCamera);
}

Actor* crt::Camera::build(const ActorBuildInfo* buildInfo) {
    return new Camera(buildInfo);
}

crt::Camera::Camera(const ActorBuildInfo* buildInfo)
    : crt::MapActor(buildInfo)
    , camera()
    , projection(10.0f, 1000.0f, 0.27f, 1.777778f)
{ }

u32 crt::Camera::onExecute() {
    this->camera.pos = this->position;
    this->camera.at = crt::Scene::instance()->map->getBonePos("W401");

    this->camera.doUpdateMatrix(&this->camera.matrix);

    return 1;
}
