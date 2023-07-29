#pragma once

#include "tsuru/save/system/customsavemgr.h"
#include "sead/idisposer.h"

class CarterraSaveMgr : public CustomSaveMgr {
public:
    struct CarterraSaveData : public CustomSaveData {
        struct SaveSlot {
            SaveSlot()
                : lastMap(1)
                , lastNode(0)
            { }

            u32 lastMap;
            u32 lastNode;
        };

        CarterraSaveData()
            : CustomSaveData()
            , saveSlots()
        { }

        SaveSlot saveSlots[3*4];
    };

public:
    SEAD_SINGLETON_DISPOSER(CarterraSaveMgr);

public:
    CarterraSaveMgr();

    u32 getSaveDataSize() override;
    void remakeSaveData() override;

    static CarterraSaveData sSaveData;
};
