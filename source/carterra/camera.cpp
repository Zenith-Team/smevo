#include "sme/carterra/camera.h"
#include "sme/carterra/scene.h"
#include "sme/carterra/map.h"
#include "sme/carterra/player.h"
#include "log.h"

namespace crt {
    REGISTER_PROFILE(Camera, ProfileID::CarterraCamera);
}

crt::Camera::Camera(const ActorBuildInfo* buildInfo)
    : crt::MapActor(buildInfo)
    , camera()
    , projection(10.0f, 1000.0f, 0.27f, 1.777778f)
{ }

u32 crt::Camera::onExecute() {
    this->camera.pos = crt::Scene::instance()->player->position + Vec3f(0.0f, 200.0f, 200.0f);
    this->camera.at = crt::Scene::instance()->player->position;

    this->camera.doUpdateMatrix(&this->camera.matrix);

    return 1;
}
