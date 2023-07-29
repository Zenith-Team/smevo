#include "tsuru/save/managers/crtsavemgr.h"
#include "tsuru/save/system/savemgrsystem.h"

SEAD_SINGLETON_DISPOSER_IMPL(CarterraSaveMgr);

CarterraSaveMgr::CarterraSaveData CarterraSaveMgr::sSaveData aligned(sead::FileDevice::sBufferMinAlignment);

CarterraSaveMgr::CarterraSaveMgr()
    : CustomSaveMgr("save://crt_savedata.dat", &CarterraSaveMgr::sSaveData)
{ }

u32 CarterraSaveMgr::getSaveDataSize() {
    return sizeof(CarterraSaveMgr::CarterraSaveData);
}

void CarterraSaveMgr::remakeSaveData() {
    CarterraSaveMgr::sSaveData = CarterraSaveData();
}

SAVEMGR_SYSTEM_ENTRY(CarterraSaveMgr);
