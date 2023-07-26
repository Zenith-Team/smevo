#pragma once

#include "game/graphics/model/modelnw.h"
#include "sme/carterra/mapactor.h"

namespace crt {

    class MapData {
    public:
        enum {
            MAGIC = 0x41324C53,
            VERSION = 1
        };
        
    public:
        struct Header {
            u32 magic;
            u32 version;
            u32 mapID; // Unique to each worldmap
        };

        struct WorldInfo {
            u32 worldID; // Used with level IDs
            char name[32];
            u32 accentColor;
        };

        struct Node {
            ENUM_CLASS(Type,
                Normal,
                Passthrough,
                Level
            );

            Type::__type__ type;
            char boneName[32];
            union {
                struct {

                } normal, passthrough;
                struct {
                    u8 levelID;
                    u32 unlocksMapID;
                } level;
            };
        };

        struct Path {
            ENUM_CLASS(Animation,
                Walk = 0, WalkSand = 1, WalkSnow = 2, WalkWet = 3,
                Jump = 4, JumpSand = 5, JumpSnow = 6, JumpWet = 7,
                Climb = 8, ClimbLeft = 9, ClimbRight = 10, Fall = 11,
                Swim = 12, Run = 13, Pipe = 14, Door = 15
            );

            Node* startingNode;
            Node* endingNode;
            f32 speed;
            Animation::__type__ animation;
            u8* unlockCriteria;
        };
    
    public:
        MapData(u8* data);

        template <typename T>
        T* fixRef(T*& indexAsPtr) {
            u32 index = (u32)indexAsPtr;
            if (index == 0xFFFFFFFF || index == 0) {
                indexAsPtr = 0;
            } else {
                indexAsPtr = (T*)(((u8*)this) + index);
            }
            
            return indexAsPtr;
        }

        Header header;
        WorldInfo worldInfo;
        u32 nodeCount;
        Node** nodes;
        u32 pathCount;
        Path** paths;
    };

    class Map : public MapActor {
        SEAD_RTTI_OVERRIDE_IMPL(Map, MapActor);
    
    public:
        Map(const ActorBuildInfo* buildInfo);
        virtual ~Map() { }

        u32 onCreate() override;
        u32 onDraw() override;
        u32 onDelete() override;

        Vec3f getBonePos(const sead::SafeString& boneName);
        MapData::Node* getNode(const sead::SafeString& name);
    
        ModelWrapper* model;
        ModelWrapper* bones;
        sead::Heap* sceneHeap;
        MapData* map;
    };

}
