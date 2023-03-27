#pragma once

#include "game/graphics/model/modelnw.h"
#include "tsuru/carterra/mapactor.h"

namespace crt {

    class MapData {
    private:
        class ResData { // Static data from the file
        public:
            enum { MAGIC = 0x41324C53 };
            
        public:
            struct Header {
                u32 magic;
                u32 version;
                u32 nodeCount;
            };

            struct WorldInfo {
                char name[32];
                u32 accentColor;
            };

            struct Node {
                ENUM_CLASS(Type,
                    Normal,
                    Passthrough,
                    Level
                );

                struct Connection {
                    ENUM_CLASS(Flags,
                        AlwaysUnlocked = 1 << 0,
                        NormalExit = 1 << 1,
                        SecretExit = 1 << 2,
                        NoReverse = 1 << 3,
                    );
                    
                    u32 node;   // Connected node ID
                    u32 flags;
                    f32 speed;  // Walking speed
                };

                u32 id;
                Type::__type__ type;
                char boneName[32];
                union {
                    struct {
                        Connection connections[4];
                    } normal, passthrough;
                    struct {
                        Connection connections[4];
                        u8 level;
                        bool hasSecretExit;
                        bool unlocksWorld;
                        u32 unlocksWorldID;
                    } level;
                };
            };
        
        public:
            ResData(u32 id);
            ~ResData();

            Header header;
            WorldInfo worldInfo;
            Node* nodes;
        };

    public:
        class Node : public ResData::Node {
        public:
            void update();

            ModelWrapper* model;
        };

    public:
        MapData(u32 id);
        ~MapData();

        ResData::Header header;
        ResData::WorldInfo worldInfo;
    };

    class Map : public MapActor {
        SEAD_RTTI_OVERRIDE_IMPL(Map, MapActor);
    
    public:
        Map(const ActorBuildInfo* buildInfo);
        virtual ~Map() { }

        static Actor* build(const ActorBuildInfo* buildInfo);

        u32 onCreate() override;
        u32 onDraw() override;
        u32 onDelete() override;

        Vec3f getBonePos(const sead::SafeString& boneName);
    
        ModelWrapper* model;
        ModelWrapper* bones;
        sead::Heap* sceneHeap;
    };

}
