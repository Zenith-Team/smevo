#include "sme/carterra/map.h"
#include "sme/carterra/scene.h"
#include "dynlibs/os/functions.h"
#include "game/resource/util.h"
#include "game/savemgr.h"
#include "sead/heapmgr.h"
#include "sead/filedevice.h"
#include "sead/filedevicemgr.h"
#include "tsuru/save/managers/crtsavemgr.h"
#include "log.h"

namespace crt {
    REGISTER_PROFILE(Map, ProfileID::CarterraMap);
}

static bool pathsUnlockedInited = false;
bool crt::Map::pathsUnlocked[512] = {false};

static void findUnlockCriteriaSize(u8* unlockCriteria, u32& idx) {
	u8 controlByte = unlockCriteria[idx++];
	u8 conditionType = controlByte >> 6;

	if (conditionType == 0) {
		u8 subConditionType = (controlByte & 0x3F);
		if (subConditionType < 4) {
			idx += 2;
        }
	} else if (conditionType == 1) {
		++idx;
	} else if (conditionType == 2 || conditionType == 3) {
		u8 termCount = (controlByte & 0x3F) + 1;
		for (int i = 0; i < termCount; i++) {
			findUnlockCriteriaSize(unlockCriteria, idx);
		}
	}
}

static bool evaluateUnlockCriteria(u8*& in) {
	u8 controlByte = *(in++);
	u8 conditionType = (controlByte >> 6);

	if (conditionType == 0) {
		u8 subConditionType = controlByte & 0x3F;
		switch (subConditionType) {
			case 15:
				return true;
			default:
				return false;
		}
	} else if (conditionType == 1) {
		bool isSecret = (controlByte & 0x10);
		u8 worldNumber = controlByte & 0xF;
		u8 levelNumber = *(in++);

		u32 conds = CarterraSaveMgr::sSaveData.saveSlots[SaveMgr::instance()->saveData->header.lastSessionSaveSlot].levelCompletions[worldNumber][levelNumber];

		if (isSecret)
			return (conds & CarterraSaveMgr::CarterraSaveData::LevelCompletion::SecretExit) != 0;
		else
			return (conds & CarterraSaveMgr::CarterraSaveData::LevelCompletion::NormalExit) != 0;
	}

	// Type: 2 = AND, 3 = OR
	bool isAnd = (conditionType == 2);
	bool isOr = (conditionType == 3);

	bool value = isOr ? false : true;

	u8 termCount = (controlByte & 0x3F) + 1;

	for (int i = 0; i < termCount; i++) {
		bool what = evaluateUnlockCriteria(in);

		if (isOr)
			value |= what;
		else if (isAnd)
			value &= what;
	}

	return value;
}

crt::MapData::MapData(u8* data) {
	this->nodeCount = 0;
	this->nodes = nullptr;
	this->pathCount = 0;
	this->paths = nullptr;

	MapData* xdata = reinterpret_cast<crt::MapData*>(data);

	this->header.magic = xdata->header.magic;
	this->header.version = xdata->header.version;
	this->header.mapID = xdata->header.mapID;

	if (this->header.magic != MapData::MAGIC) {
		PRINT("Invalid magic! Found: ", fmt::hex, this->header.magic, " Expected: ", fmt::hex, (u32)MapData::MAGIC);
        return;
    }

	if (this->header.version != MapData::VERSION) {
        PRINT("Invalid version! Found: ", fmt::hex, this->header.version, " Expected: ", fmt::hex, (u32)MapData::VERSION);
		return;
    }

	this->worldInfo.worldID = xdata->worldInfo.worldID;
	memcpy(&this->worldInfo.name, xdata->worldInfo.name, 32);
	this->worldInfo.accentColor = xdata->worldInfo.accentColor;

	this->nodeCount = xdata->nodeCount;
	this->pathCount = xdata->pathCount;

	xdata->fixRef(xdata->nodes);
	xdata->fixRef(xdata->paths);

	this->nodes = new Node*[this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		xdata->fixRef(xdata->nodes[i]);

		this->nodes[i] = new Node;
		this->nodes[i]->type = xdata->nodes[i]->type;
		memcpy(&this->nodes[i]->boneName, &xdata->nodes[i]->boneName, 32);

		if (this->nodes[i]->type == MapData::Node::Type::Level) {
			this->nodes[i]->level.levelID = xdata->nodes[i]->level.levelID;
			this->nodes[i]->level.unlocksMapID = xdata->nodes[i]->level.unlocksMapID;
		}
	}

	this->paths = new Path*[this->pathCount];
	for (u32 i = 0; i < this->pathCount; i++) {
		xdata->fixRef(xdata->paths[i]);

		this->paths[i] = new Path;
		xdata->fixRef(xdata->paths[i]->startingNode);
		xdata->fixRef(xdata->paths[i]->endingNode);

		this->paths[i]->startingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->startingNode == xdata->nodes[j]) {
				this->paths[i]->startingNode = this->nodes[j];
            }
        }

		this->paths[i]->endingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->endingNode == xdata->nodes[j]) {
				this->paths[i]->endingNode = this->nodes[j];
            }
        }

		this->paths[i]->speed = xdata->paths[i]->speed;
		this->paths[i]->animation = xdata->paths[i]->animation;

		xdata->fixRef(xdata->paths[i]->unlockCriteria);

		u32 ucSize = 0;
		u8* unlockCriteria = xdata->paths[i]->unlockCriteria;
		findUnlockCriteriaSize(unlockCriteria, ucSize);

		this->paths[i]->unlockCriteria = new u8[ucSize];
		memcpy(this->paths[i]->unlockCriteria, unlockCriteria, ucSize);
	}
}

crt::Map::Map(const ActorBuildInfo* buildInfo)
    : crt::MapActor(buildInfo)
    , model(nullptr)
    , bones(nullptr)
    , sceneHeap((sead::Heap*)buildInfo->rotation) // Reuse the rotation field to pass the scene heap
{
    sead::HeapMgr::instance()->setCurrentHeap(this->sceneHeap);

    char name[32] = {0};
    char modelPath[32] = {0};
    char dataPath[32] = {0};
    __os_snprintf(name, 32, "CS_W%d", this->settings1);
    __os_snprintf(modelPath, 64, "course_select/%s.szs", name);
    __os_snprintf(dataPath, 64, "course_select/%s.a2ls", name);

    loadResource(name, modelPath);

    sead::FileHandle handle;
    sead::FileDeviceMgr::instance()->tryOpen(&handle, dataPath, sead::FileDevice::FileOpenFlag_ReadOnly, 0);

    u32 fileSize = handle.getFileSize();

    u8* data = (u8*)this->sceneHeap->tryAlloc(fileSize, sead::FileDevice::sBufferMinAlignment);

    if (data == nullptr) {
        PRINT("Failed to allocate memory for map: ", dataPath);
        return;
    }

    u32 bytesRead = handle.read(data, fileSize);

    this->map = new MapData(data);

    delete[] data;

	if (pathsUnlockedInited == false) {
		PRINT("Initing pathsUnlocked");

		for (u32 i = 0; i < this->map->pathCount; i++) {
        	Map::pathsUnlocked[i] = evaluateUnlockCriteria(this->map->paths[i]->unlockCriteria);
		}

		pathsUnlockedInited = true;
	}
}

u32 crt::Map::onCreate() {
    sead::HeapMgr::instance()->setCurrentHeap(this->sceneHeap);

    char name[32] = {0};
    char terrainModelName[32] = {0};
	char synchroModelName[32] = {0};

    __os_snprintf(name, 32, "CS_W%d", this->settings1);
    __os_snprintf(terrainModelName, 32, "CS_W%d_World", this->settings1);
	__os_snprintf(synchroModelName, 32, "CS_W%d_Synchro", this->settings1);

    this->model = ModelWrapper::create(name, terrainModelName);
    this->bones = ModelWrapper::create(name, name);
	this->synchro = ModelWrapper::create(name, synchroModelName, 1);

	this->synchro->playSklAnim(synchroModelName);

    Mtx34 mtx;
    mtx.makeRTIdx(0, this->position);

    this->bones->setMtx(mtx);
    this->bones->setScale(0.1f);
    this->bones->updateModel();

    this->model->setMtx(mtx);
    this->model->setScale(0.1f);
    this->model->updateModel();

	this->synchro->setMtx(mtx);
	this->synchro->setScale(0.1f);

    return 1;
}

u32 crt::Map::onExecute() {
	this->synchro->updateAnimations();
	this->synchro->updateModel();

	return 1;
}

u32 crt::Map::onDraw() {
    this->model->draw();
	this->synchro->draw();

    return 1;
}

u32 crt::Map::onDelete() {
    delete this->model;
    delete this->bones;
	delete this->synchro;
    
    return 1;
}

Vec3f crt::Map::getBonePos(const sead::SafeString& name) {
    Mtx34 mtx;
    this->bones->model->getBoneWorldMatrix(this->bones->model->searchBoneIndex(name), &mtx);

    return Vec3f(mtx.m[0][3], mtx.m[1][3], mtx.m[2][3]);
}

crt::MapData::Node* crt::Map::getNode(const sead::SafeString& name) {
	for (u32 i = 0; i < this->map->nodeCount; i++) {
		if (strcmp(name.cstr(), this->map->nodes[i]->boneName)) {
			return this->map->nodes[i];
		}
	}

	PRINT("ERROR: Unable to find node: ", name.cstr());

	return nullptr;
}

void crt::Map::evalPaths() {
	for (u32 i = 0; i < this->map->pathCount; i++) {
		if (Map::pathsUnlocked[i]) {
			PRINT("Path ", i, " already unlocked!");
			continue;
		}

        Map::pathsUnlocked[i] = evaluateUnlockCriteria(this->map->paths[i]->unlockCriteria);
    
		if (Map::pathsUnlocked[i]) {
			PRINT("New path unlocked! ", i);
		} else {
			PRINT("Path ", i, " still locked!");
		}
	}
}
