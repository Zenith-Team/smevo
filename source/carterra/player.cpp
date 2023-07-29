#include "sme/carterra/player.h"
#include "sme/carterra/scene.h"
#include "sme/carterra/camera.h"
#include "game/task/taskmgr.h"
#include "game/globalstatekeeper.h"
#include "tsuru/save/managers/tsurusavemgr.h"
#include "log.h"
#include "imgui/imgui.h"

namespace crt {
    REGISTER_PROFILE(Player, ProfileID::CarterraPlayer);

    CREATE_STATE(Player, Idle);
    CREATE_STATE(Player, Walk);
    CREATE_STATE(Player, EnterLevel);
}

crt::Player::Player(const ActorBuildInfo* buildInfo)
    : MapActor(buildInfo)
    , modelMario(0, 1, 0, true)
    , currentNode(nullptr)
    , currentPath(nullptr)
    , states(this)
{ }

u32 crt::Player::onCreate() {
    this->currentNode = crt::Scene::instance()->map->map->nodes[0];
    
    this->states.changeState(&crt::Player::StateID_Idle);

    return this->onExecute();
}

u32 crt::Player::onExecute() {
    this->states.execute();

    Mtx34 mtx;
    mtx.makeSRTIdx(0.25f, this->rotation, this->position);
    
    this->modelMario.playerModel->setMtx(mtx);
    this->modelMario.playerModel->update();

    sead::Mathu::chase(&this->rotation.y, fixDeg(this->targetRotation), fixDeg(20.0f));

    return 1;
}

u32 crt::Player::onDraw() {
    this->modelMario.draw();
    
    return 1;
}

/** STATE: Idle */

void crt::Player::beginState_Idle() {
    this->modelMario.playAnim(CharacterModelMgr::Animation::CSIdle);
}

void crt::Player::executeState_Idle() {
    InputControllers* controllers = &crt::Scene::instance()->controllers;
    InputControllers::ControllerID::__type__ ctrl = InputControllers::ControllerID::Gamepad;

    MapData* map = crt::Scene::instance()->map->map;

    if (controllers->buttonA(ctrl) && this->currentNode->type == MapData::Node::Type::Level) {
        this->states.changeState(&StateID_EnterLevel);
    }

    if (controllers->buttonRight(ctrl) || controllers->buttonUp(ctrl) || controllers->buttonLeft(ctrl) || controllers->buttonDown(ctrl)) {
        f32 angle;

        if (controllers->buttonLeft(ctrl)) {
            angle = 270.0f;
        }

        if (controllers->buttonDown(ctrl)) {
            angle = 0.0f;
        }

        if (controllers->buttonRight(ctrl)) {
            angle = 90.0f;
        }

        if (controllers->buttonUp(ctrl)) {
            angle = 180.0f;
        }

        for (u32 i = 0; i < map->pathCount; i++) {
            if (map->paths[i]->startingNode != this->currentNode && map->paths[i]->endingNode != this->currentNode) {
                continue;
            }

            MapData::Node* otherNode = map->paths[i]->startingNode == this->currentNode ? map->paths[i]->endingNode : map->paths[i]->startingNode;
            Vec3f otherNodePos = crt::Scene::instance()->map->getBonePos(otherNode->boneName);

            f32 otherNodeAngle = radToDeg(atan2f(otherNodePos.x - this->position.x, otherNodePos.z - this->position.z));

            if (otherNodeAngle < 0.0f) {
                otherNodeAngle += 360.0f;
            }

            if (otherNodeAngle > 360.0f) {
                otherNodeAngle -= 360.0f;
            }

            if (sead::Mathf::abs(angle - otherNodeAngle) < 80.0f) {
                this->currentPath = map->paths[i];
                this->targetRotation = otherNodeAngle;
                break;
            }
        }

        if (this->currentPath != nullptr) {
            this->states.changeState(&crt::Player::StateID_Walk); 
        } else {
            this->targetRotation = angle;
            this->rotation.y = fixDeg(angle);
        }
    }
}

void crt::Player::endState_Idle() { }

/** STATE: Walk */

void crt::Player::beginState_Walk() {
    this->modelMario.playAnim(CharacterModelMgr::Animation::Run3);
}

void crt::Player::executeState_Walk() {
    MapData::Node* otherNode = this->currentPath->startingNode == this->currentNode ? this->currentPath->endingNode : this->currentPath->startingNode;
    Vec3f targetPos = crt::Scene::instance()->map->getBonePos(otherNode->boneName);
    Vec3f nodeDir = targetPos - this->position;
    nodeDir.normalize();

    this->position += nodeDir * this->currentPath->speed * 0.5f;

    if (this->position.distanceTo(targetPos) < (0.2f * this->currentPath->speed)) {
        if (otherNode->type == MapData::Node::Type::Passthrough) {
            MapData::Path* nextPath = this->currentPath;

            for (u32 i = 0; i < crt::Scene::instance()->map->map->pathCount; i++) {
                MapData::Path* checkPath = crt::Scene::instance()->map->map->paths[i];

                if (checkPath == this->currentPath) {
                    continue;
                }

                if (checkPath->startingNode == otherNode || checkPath->endingNode == otherNode) {
                    nextPath = checkPath;

                    MapData::Node* nextNode = nextPath->startingNode == otherNode ? nextPath->endingNode : nextPath->startingNode;
                    Vec3f nextNodePos = crt::Scene::instance()->map->getBonePos(nextNode->boneName);
                    this->targetRotation = radToDeg(atan2f(nextNodePos.x - this->position.x, nextNodePos.z - this->position.z));

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

/** STATE: EnterLevel */

void crt::Player::beginState_EnterLevel() {
    this->modelMario.playAnim(CharacterModelMgr::Animation::CSEnterLevel);

    this->targetRotation = 0;
}

void crt::Player::executeState_EnterLevel() {
    static int i = 60;

    if (--i == 0) {
        i = 60;

        Vec2f faderPos;
        const sead::Viewport vp(0.0f, 0.0f, 1280.0f, 720.0f);
        crt::Scene::instance()->camera->camera.projectByMatrix(&faderPos, this->position, crt::Scene::instance()->camera->projection, vp);
        TaskMgr::instance()->faderPos = faderPos;

        GlobalStateKeeper* gs = GlobalStateKeeper::instance();
        
        // Level numbers used for name text
        gs->_1A8 = this->currentNode->level.levelID-1;
        gs->_1B8 = this->currentNode->level.levelID-1;

        TaskMgr::instance()->startLevel(crt::Scene::instance(), crt::Scene::instance()->map->map->worldInfo.worldID-1, this->currentNode->level.levelID-1);
    }
}

void crt::Player::endState_EnterLevel() { }
