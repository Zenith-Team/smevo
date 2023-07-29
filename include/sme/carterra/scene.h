#pragma once

#include "sead/task.h"
#include "sme/carterra/renderer.h"
#include "tsuru/inputcontrollers.h"
#include "game/layout/layoutcontainer.h"

namespace crt {

    class Camera;
    class Map;
    class Player;

    class MapUILayout : public LayoutContainer {
    public:
        MapUILayout()
            : LayoutContainer(this->animators, sizeof(this->animators) / sizeof(this->animators[0]))
        { }

        virtual ~MapUILayout() { }

        LayoutAnimator animators[1];
    };

    class Scene : public sead::CalculateTask {
        SEAD_RTTI_OVERRIDE_IMPL(Scene, sead::CalculateTask);
        SEAD_SINGLETON_TASK(Scene);

    public:
        Scene(const sead::TaskConstructArg& arg);
        virtual ~Scene();

        static sead::TaskBase* construct(const sead::TaskConstructArg& arg);

        void prepare() override;
        void enter() override;
        void calc() override;

        crt::Renderer renderer;
        crt::Camera* camera;
        crt::Player* player;
        crt::Map* map;
        InputControllers controllers;
        MapUILayout uiLayout;
    };

}
