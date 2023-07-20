#include "sme/carterra/player.h"
#include "sme/carterra/scene.h"
#include "game/task/taskmgr.h"
#include "tsuru/save/managers/tsurusavemgr.h"
#include "log.h"

namespace crt {
    REGISTER_PROFILE(Player, ProfileID::CarterraPlayer);

    CREATE_STATE(Player, Idle);
    CREATE_STATE(Player, Walk);
}

PROFILE_RESOURCES(ProfileID::CarterraPlayer, Profile::LoadResourcesAt::CourseSelect, "MarioMdl");

crt::Player::Player(const ActorBuildInfo* buildInfo)
    : MapActor(buildInfo)
    , model(nullptr)
    , currentNode(nullptr)
    , currentPath(nullptr)
    , states(this)
{ }

u32 crt::Player::onCreate() {
    this->model = ModelWrapper::create("MarioMdl", "MB_model");
    this->currentNode = crt::Scene::instance()->map->map->nodes[0];
    
    this->states.changeState(&crt::Player::StateID_Idle);

    return 1;
}

u32 crt::Player::onExecute() {
    this->states.execute();

    Mtx34 mtx;
    mtx.makeRTIdx(this->rotation, this->position);
    
    this->model->setMtx(mtx);
    this->model->setScale(0.25f);
    this->model->updateModel();

    sead::Mathu::chase(&this->rotation.y, fixDeg(this->targetRotation), fixDeg(10.0f));

    return 1;
}

u32 crt::Player::onDraw() {
    this->model->draw();
    
    return 1;
}

/** STATE: Idle */

void crt::Player::beginState_Idle() { }

void crt::Player::executeState_Idle() {
    InputControllers* controllers = &crt::Scene::instance()->controllers;
    InputControllers::ControllerID::__type__ ctrl = InputControllers::ControllerID::Gamepad;

    MapData* map = crt::Scene::instance()->map->map;

    if (controllers->buttonA(ctrl) && this->currentNode->type == MapData::Node::Type::Level) {
        TaskMgr::instance()->startLevel(crt::Scene::instance(), 1, this->currentNode->level.levelID-1);
    }

    if (controllers->buttonRight(ctrl) || controllers->buttonUp(ctrl) || controllers->buttonLeft(ctrl) || controllers->buttonDown(ctrl)) {
        f32 angle;

        if (controllers->buttonLeft(ctrl)) {
            angle = 0.0f;
        }

        if (controllers->buttonDown(ctrl)) {
            angle = 90.0f;
        }

        if (controllers->buttonRight(ctrl)) {
            angle = 180.0f;
        }

        if (controllers->buttonUp(ctrl)) {
            angle = 270.0f;
        }

        MapData::Path* path = map->paths[0];
        f32 prevAngle = 99999999.0f;

        for (u32 i = 0; i < map->pathCount; i++) {
            if (map->paths[i]->startingNode != this->currentNode && map->paths[i]->endingNode != this->currentNode) {
                continue;
            }

            MapData::Node* otherNode = map->paths[i]->startingNode == this->currentNode ? map->paths[i]->endingNode : map->paths[i]->startingNode;
            Vec3f otherNodePos = crt::Scene::instance()->map->getBonePos(otherNode->boneName);

            f32 otherNodeAngle = radToDeg(atan2f(otherNodePos.x - this->position.x, otherNodePos.z - this->position.z));

            if (abs(angle - otherNodeAngle) < abs(angle - prevAngle)) {
                path = map->paths[i];
                prevAngle = otherNodeAngle;
            }
        }

        this->currentPath = path;
        this->targetRotation = prevAngle;

        this->states.changeState(&crt::Player::StateID_Walk); 
    }

}

void crt::Player::endState_Idle() { }

/** STATE: Walk */

void crt::Player::beginState_Walk() { }

void crt::Player::executeState_Walk() {
    MapData::Node* otherNode = this->currentPath->startingNode == this->currentNode ? this->currentPath->endingNode : this->currentPath->startingNode;
    Vec3f targetPos = crt::Scene::instance()->map->getBonePos(otherNode->boneName);
    Vec3f nodeDir = targetPos - this->position;
    nodeDir.normalize();

    this->position += nodeDir * 0.4f;

    if (this->position.distanceTo(targetPos) < 0.1f) {
        if (otherNode->type == MapData::Node::Type::Passthrough) {
            MapData::Path* nextPath = this->currentPath;

            for (u32 i = 0; i < crt::Scene::instance()->map->map->pathCount; i++) {
                MapData::Path* checkPath = crt::Scene::instance()->map->map->paths[i];

                if (checkPath == this->currentPath) {
                    continue;
                }

                if (checkPath->startingNode == otherNode || checkPath->endingNode == otherNode) {
                    nextPath = checkPath;
                    break;
                }
            }

            this->currentPath = nextPath;
            this->currentNode = otherNode;
            return;
        }

        this->currentNode = otherNode;
        this->currentPath = nullptr;
        this->states.changeState(&crt::Player::StateID_Idle);
    }
}

void crt::Player::endState_Walk() { }
