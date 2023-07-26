#pragma once

#include "game/resource/resarchive.h"
#include "game/graphics/model/modelnw.h"

class CharacterModel : public sead::IDisposer { // Size: 0xF0
public:
    CharacterModel(const sead::SafeString& tex, const sead::SafeString& anim);
    virtual ~CharacterModel();

    SEAD_RTTI_BASE(CharacterModel);

    virtual void vf24() = 0;
    virtual void vf2C() = 0;
    virtual void vf34() = 0;
    virtual void vf3C() = 0;
    virtual void vf44() = 0;
    virtual void vf4C() = 0;
    virtual void vf54() = 0;
    virtual void vf5C() = 0;
    virtual void vf64() = 0;
    virtual void vf6C() = 0;
    virtual void vf74() = 0;
    virtual f32 vf7C(u32);
    virtual f32 vf84(u32);
    virtual void vf8C(u32, f32, u32, u32);
    virtual void vf94() = 0;
    virtual void vf9C() = 0;
    virtual void vfA4();
    virtual void vfAC();
    virtual void vfB4();
    virtual void vfBC();
    virtual void update() = 0;

    void setMtx(const Mtx34& mtxSRT);

    ResArchive* modelResArchive;
    ResArchive* animResArchive;
    ModelWrapper* bodyModel;
    s32 _1C;
    s32 _20;
    s32 _24;
    u32 _28;
    Mtx34 srtMtx;  // Set externally
    Mtx34 srtMtx2; // Copies from srtMtx
    Vec3f _8C;
    Vec3f _98;
    u32 currentCharacter;
    u32 _A8;
    u32 _AC;
    u32 _B0;
    f32 _B4;
    f32 _B8;
    f32 _BC;
    f32 _C0;
    u32 animFlagsArray[3];
    u32 _D0;
    u32 _D4; // Index related to jump anim
    u32 _D8;
    u32 _DC;
    u32 _E0;
    u32 _E4;
    u8 _E8[4]; // Unknown values
    u8 _EC;
};

static_assert(sizeof(CharacterModel) == 0xF0, "CharacterModel size mismatch");

class YoshiModel : public CharacterModel {
public:
    // All green in vanilla
    ENUM_CLASS(TexColor,
        TexColor_Green,
        TexColor_Pink,
        TexColor_LightBlue,
        TexColor_Yellow
    );

public:
    YoshiModel(YoshiModel::TexColor::__type__ color);
    virtual ~YoshiModel();

    static sead::SafeString texArray[];

    u32 _F0;
    u8 _F4[0x40]; // Is class
    u32 _134;
    f32 _138;
    u8 _13C;
    YoshiModel::TexColor::__type__ color;
    // ...
};

class PlayerModel : public CharacterModel {
public:
    struct ResInfo {
        sead::SafeString modelResName;
        sead::SafeString animResName;
        sead::SafeString normalBodyName;
        sead::SafeString smallBodyName;
        sead::SafeString propellerBodyName;
        sead::SafeString penguinBodyName;
        sead::SafeString squirrelBodyName;
        sead::SafeString normalHeadName;
        sead::SafeString smallHeadName;
        sead::SafeString propellerHeadName;
        sead::SafeString penguinHeadName;
        sead::SafeString squirrelHeadName;
    };

    ENUM_CLASS(PowerupModel,
        Normal,
        Small,
        Propeller,
        Penguin,
        Squirrel
    );

public:
    PlayerModel(PlayerModel::ResInfo* resInfo, u32 character, u32, u32, bool useLightMaps);
    virtual ~PlayerModel();

    SEAD_RTTI_OVERRIDE(PlayerModel, CharacterModel);

    // TODO: vf overrides
    
    void changePowerup(u32 powerupState); // Uses PlayerBase enum

    ModelWrapper* bodyModels[5];
    ModelWrapper* headModels[5];
    PlayerModel::ResInfo* resInfo;
    u8 _11C[0x120 - 0x11C]; //? unknown values
    PlayerModel::PowerupModel::__type__ prevPowerupModel;
    PlayerModel::PowerupModel::__type__ currentPowerupModel;
    sead::BitFlag16 _128;
    u8 _12C[96];
    u32 _18C;
    u8 _190[64];
    u32 _1D0;
    u32 _1D4;
    u32 _1D8;
    u32 _1DC;
    u32 _1E0;
    u32 _1E4;
    u32 _1E8;
    u8 _1EC;
    u32 _1F0;
    void* _1F4;
    f32 _1F8;
    f32 _1FC;
    f32 _200;
    f32 _204;
    u32 _208;
    f32 _20C;
    sead::BitFlag32 _210;
    bool useLightMaps;
};

static_assert(sizeof(PlayerModel) == 0x218, "PlayerModel size mismatch");

class BrosModel : public PlayerModel { // Size: 0x240
public:
    BrosModel(u32 character, u32, u32, bool useLightMaps);
    virtual ~BrosModel();

    SEAD_RTTI_OVERRIDE(BrosModel, PlayerModel);

    void update() override;

    ResArchive* luigiAnimArchive;
    u8 _21C[0x240 - 0x21C]; //? unknown values
};

static_assert(sizeof(BrosModel) == 0x240, "BrosModel size mismatch");
