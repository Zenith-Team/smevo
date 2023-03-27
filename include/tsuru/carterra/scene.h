#pragma once

#include "sead/task.h"
#include "tsuru/carterra/renderer.h"
#include "tsuru/inputcontrollers.h"

namespace crt {

    class Camera;
    class Map;

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

        Renderer renderer;
        InputControllers controllers;
        Camera* camera;
        Map* map;
    };

}
