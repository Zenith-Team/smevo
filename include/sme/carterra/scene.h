#pragma once

#include "sead/task.h"
#include "sme/carterra/renderer.h"
#include "tsuru/inputcontrollers.h"

namespace crt {

    class Camera;
    class Map;
    class Player;

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
    
    private:
        friend class Camera;

        crt::Renderer renderer;
        crt::Camera* camera;
        crt::Player* player;

    public:
        InputControllers controllers;
        crt::Map* map;
    };

}
