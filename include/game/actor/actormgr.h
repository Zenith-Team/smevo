#pragma once

#include <sead.h>
#include <game/actor/stage/stageactor.h>

class ActorBuffer { // Size: 0x18
public:
    ActorBuffer();

    // Locates an actor by the Actor ID
    // @param id Actor ID of the intended target actor
    // @return Pointer to located actor
    Actor* findActorByID(const u32& id);

    sead::Buffer<Actor*> start;
    sead::Buffer<Actor*> end;
    u32 _10;
    u8  _14;
};

class ActorMgr { // Size: 0x6A44
    SEAD_SINGLETON_DISPOSER(ActorMgr)

public:
    // Spawns an actor in the current level
    // @param buildInfo ActorBuildInfo to build the actor with
    // @return Pointer to created actor
    Actor* create(ActorBuildInfo& buildInfo, u32 addToActive);

    // Creates an instance of an actor class using the build info
    // @param buildInfo ActorBuildInfo to build the actor with
    // @return Pointer to created actor
    Actor* instanciateActor(ActorBuildInfo& buildInfo, bool dontDefer);

    // Calls "create" virtual functions
    // @param Pointer to actor to call the functions on
    u32 createActor(Actor* actor);

    // Calls "execute" virtual functions
    void executeActors();

    sead::UnitHeap* playerUnitHeap;
    sead::UnitHeap* actorUnitHeap;
    u8 deferredActorCreations[0x5970]; // sead::FixedRingBuffer<ActorBuildInfo, 520>
    sead::OffsetList<Actor> actorsToCreate;
    sead::OffsetList<Actor> actorsToDelete;
    sead::OffsetList<Actor> activeActors;
    sead::OffsetList<Actor> drawableActors;
    sead::FixedPtrArray<sead::Heap, 520> deletedActorHeaps;
    sead::FixedPtrArray<Actor, 520> finalExecuteList;
    ActorBuffer actors;
    u32 currentID;
    bool currentWasNotDeferred;
    // ...
};
