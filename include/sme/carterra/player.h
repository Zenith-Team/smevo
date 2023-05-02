#pragma once

#include "sme/carterra/mapactor.h"
#include "sme/carterra/map.h"
#include "game/graphics/model/modelnw.h"
#include "game/states.h"

namespace crt {

    class Player : public MapActor {
        SEAD_RTTI_OVERRIDE_IMPL(Player, MapActor);

    public:
        Player(const ActorBuildInfo* buildInfo);
        virtual ~Player() { }

        u32 onCreate() override;
        u32 onExecute() override;
        u32 onDraw() override;

        ModelWrapper* model;
        MapData::Node* currentNode;
        MapData::Path* currentPath;
        MultiStateWrapper<crt::Player> states;
        f32 targetRotation;

        DECLARE_STATE(Player, Idle);
        DECLARE_STATE(Player, Walk);
    };

}
