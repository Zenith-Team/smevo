#include "tsuru/atlys/player.h"
#include "tsuru/atlys/scene.h"
#include "math/functions.h"
#include "game/util.h"
#include "game/task/taskmgr.h"

CREATE_STATE(Atlys::Player, Idle);
CREATE_STATE(Atlys::Player, Walking);

const Profile AtlysPlayerProfile(&Atlys::Player::build, ProfileID::AtlysPlayer);
PROFILE_RESOURCES(ProfileID::AtlysPlayer, "kanibo");

Atlys::Player::Player(const ActorBuildInfo* buildInfo)
    : Atlys::Actor(buildInfo)
    , states(this)
    , model(nullptr)
    , currentNode(nullptr)
    , targetNode(nullptr)
    , tex()
{ }

Actor* Atlys::Player::build(const ActorBuildInfo* buildInfo) {
    return new Atlys::Player(buildInfo);
}

u32 Atlys::Player::onCreate() {
    this->currentNode = Atlys::Scene::instance()->map->findNodeByID(0);
    this->position = this->currentNode->position;
    this->states.changeState(&Atlys::Player::StateID_Idle);

    return 1;
}

u32 Atlys::Player::onExecute() {
    this->states.execute();

    if (this->rotation != this->targetRotation)
        moveFloatTo(this->rotation, this->targetRotation, 0.1f);

    return 1;
}

u32 Atlys::Player::onDraw() {
    DrawMgr::instance()->drawModel(this->model);

    return 1;
}

void Atlys::Player::updateTargetRotation() {
    this->targetRotation = radToDeg(atan2f(this->targetNode->position.y - this->position.y, this->targetNode->position.x - this->position.x));
}

void Atlys::Player::findTargetNode(Direction::DirectionType direction) {
    Map::Node* target = nullptr;

    if (this->currentNode->type == Map::Node::Type_Normal && this->currentNode->Normal_connections[direction].node != ATLYS_NODE_INVALID)
        target = Atlys::Scene::instance()->map->findNodeByID(this->currentNode->Normal_connections[direction].node);
    else if (this->currentNode->type == Map::Node::Type_Level && this->currentNode->Level_connections[direction].node != ATLYS_NODE_INVALID)
        target = Atlys::Scene::instance()->map->findNodeByID(this->currentNode->Level_connections[direction].node);

    if (!target)
        return;

    if (this->currentNode->type == Map::Node::Type_Normal && this->currentNode->Normal_connections[direction].node != ATLYS_NODE_INVALID)
        this->walkingSpeed = this->currentNode->Normal_connections[direction].speed;
    else if (this->currentNode->type == Map::Node::Type_Level && this->currentNode->Level_connections[direction].node != ATLYS_NODE_INVALID)
        this->walkingSpeed = this->currentNode->Level_connections[direction].speed;

    this->targetNode = target;    
    this->states.changeState(&Atlys::Player::StateID_Walking);
    this->updateTargetRotation();
}

/* STATE: Idle */

void Atlys::Player::beginState_Idle() { }

void Atlys::Player::executeState_Idle() {
    const InputControllers& controllers = Atlys::Scene::instance()->controllers;

    if (controllers.buttonA(Atlys::Scene::instance()->activeController) && this->currentNode->type == Map::Node::Type_Level)
        TaskMgr::instance()->startLevel(Atlys::Scene::instance(), this->currentNode->world, this->currentNode->level);

    // Find the target node
    if (controllers.buttonRight(Atlys::Scene::instance()->activeController))      this->findTargetNode(Direction::Right);
    else if (controllers.buttonLeft(Atlys::Scene::instance()->activeController))  this->findTargetNode(Direction::Left);
    else if (controllers.buttonUp(Atlys::Scene::instance()->activeController))    this->findTargetNode(Direction::Up);
    else if (controllers.buttonDown(Atlys::Scene::instance()->activeController))  this->findTargetNode(Direction::Down);
}

void Atlys::Player::endState_Idle() { }

/* STATE: Walking */

void Atlys::Player::beginState_Walking() {
    if (!this->targetNode) {
        LOG("Walk to target node failed because nullptr");
        this->states.changeState(&Atlys::Player::StateID_Idle);
    }
}

void Atlys::Player::executeState_Walking() {
    if (!this->targetNode) {
        LOG("Walk to target node failed because nullptr");
        return;
    }

    LOG("Walking to node at %f, %f, current position: %f, %f", this->targetNode->position.x, this->targetNode->position.y, this->position.x, this->position.y);
    
    // Pythagorean theorem
    f32 distance = sqrtf(powf(this->targetNode->position.x - this->position.x, 2) + powf(this->targetNode->position.y - this->position.y, 2));

    if (distance > 1.0f) {
        // TODO: Use normalized vector instead
        this->position.x += (this->targetNode->position.x - this->position.x) / distance * this->walkingSpeed;
        this->position.y += (this->targetNode->position.y - this->position.y) / distance * this->walkingSpeed;
    } else { // Reached target
        this->position = this->targetNode->position;
        
        if (this->targetNode->type == Map::Node::Type_Passthrough) {
            // If we reached the target and the type is passthrough, instead of stopping change target to the next node
            if (this->targetNode->Passthrough_connections[0].node == this->currentNode->id)
                this->targetNode = Atlys::Scene::instance()->map->findNodeByID(this->targetNode->Passthrough_connections[1].node);
            else
                this->targetNode = Atlys::Scene::instance()->map->findNodeByID(this->targetNode->Passthrough_connections[0].node);

            this->updateTargetRotation();
        } else {
            LOG("Reached target node");
            this->states.changeState(&Atlys::Player::StateID_Idle);
        }
    }
}

void Atlys::Player::endState_Walking() {
    this->currentNode = this->targetNode;
    this->position = this->currentNode->position;
    this->targetNode = nullptr;
    this->walkingSpeed = 0.0f;
}
