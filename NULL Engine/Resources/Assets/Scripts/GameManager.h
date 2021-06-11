#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"
#include "Timer.h"

#include "Player.h"
#include "Grogu.h"

#define BACKTRACK 5

class ParsoNode;
class GameObject;
class Gate;
class CameraMovement;
class DialogManager;
class C_AudioSource;

class PerfectTimer;

struct ItemData;

struct FixedLevelNames
{
    std::string hub = "HUB";
    std::string l1Initial = "InitialL1";
    std::string l1Shop = "ShopL1";
    std::string ruinsShop = "Shop_Ruins";
    std::string l1Boss = "BossL1";
    std::string ruinsBoss = "Boss_Ruins";
    std::string winScene = "WinScene";
    std::string loseScene = "LoseScene";
};

class StoryDialogData
{
public:
    bool visitedHUB = false;            //When mando enters the HUB for the first time a cinematic + dialog should happen
    bool defeatedIG11FirstTime = false; //When Mando hasn't defeated IG-11 Grogu should not be with him and IG-11 should use a special dialog
    bool defeatedIG12FirstTime = false;
    bool talkedToArmorer = false;
    bool firstTimeHub = false;
    bool talkedToGrogu = false;

    void Save(ParsonNode *node);
    void Load(ParsonNode *node);
};

class RunStats
{
public:

    uint attempt = 0;
    uint runKills = 0;
    float runPrecision = 0.f;
    float runTime = 0;                  //In seconds (uses dt)
    std::string weaponUsed = "Rifle";

    void Save(ParsonNode* node);
    void Load(ParsonNode* node);
    void ResetRun();
};


class SCRIPTS_API GameManager : public Script 
{
public:

    GameManager();
    ~GameManager();
    void Awake() override;
    void Start()override;
    void Update()override; 
    void CleanUp() override;

    void OnCollisionEnter(GameObject* object) override;

    void GenerateNewRun(bool fromMenu);
    void GoNextRoom();
    void InitiateLevel(int level);
    void Continue();
    void ReturnHub();
    void ReturnToMainMenu();

    void Pause();
    void Resume();

    std::vector<ItemData*> GetChestItemPool() const { return chestItemPool; };
    std::vector<ItemData*> GetShopItemPool() const { return shopItemPool; };
    std::vector<ItemData*> GetHubItemPool() const { return hubItemPool; };

    //Dialog & Story funtions
    void KilledIG11(int bossNum);
    void TalkedToArmorer();
    void BoughtFromArmorer();

    void SetUpWinScreen();

    //Called from main menu newgame button
    void ResetArmorerItemsLvl();

private:
    //ArmorerItems
    void SaveArmorerItemLvl(ParsonNode& node);
    void LoadArmorerItemLvl(ParsonNode& node);
    //Level Generator
    void GenerateLevel();
    void GoPreviousRoom();
    void AddFixedRoom(std::string name, int level, int position);
    void HandleRoomGeneration();

    void SaveManagerState();

    void BackTrackUpdate();
    void GateUpdate();

    void DropChest();

    void HandleBackgroundMusic();

public:
    std::vector<std::string> level1;
    int l1Easy = -1;
    int l1Intermediate = -1;
    std::vector<std::string> level1Ruins;
    int RuinsEasy = -1;
    int RuinsIntermediate = -1;

    bool enabled;
    std::string mainMenuScene;
    std::string SpawnPointName;
    std::string gateName = "NextSceneTrigger";
    std::string loseSceneName = "Assets/Scenes/LoseScene.json";

    Prefab playerPrefab;
    Player* playerScript = nullptr;

    Prefab groguPrefab;
    Grogu* groguScript = nullptr;

    //Room Name Vars
    FixedLevelNames levelNames;
    
    
    Prefab mistPlane1;
    Prefab mistPlane2;
    float3 mistPlane1Position = float3::zero;
    float3 mistPlane2Position = float3::zero;

    DialogManager* dialogManager = nullptr;

    Prefab chestPrefab;
    int chestSpawnChance = 60;

    std::string cameraName = "GameCameraVS2";

    bool doCameraCutscene = false;

    //Leave boss rooms timer
    void PickedItemUp();

    void UpdateLeaveBoss();

    //I'm trash
    bool tooBad = false;
    bool awaitingChestDrop = true;
    bool killedBoss = false;
    bool pickedItemUp = false; //If it has picked the item up
    bool wantToLeaveBoss = false; //When it starts timer to leave
    bool droppedChest = false; //If boss has dropped chest or not
    float leaveBossTimer = 0.f;
    float leaveBossDelay = 5.f;

    //Story & dialog vars
    StoryDialogData storyDialogState;

    bool paused = false;

    //Win screen stats
    RunStats runStats;

    bool cameraShake = true;

private:

    GameObject* cameraGameObject = nullptr;
    CameraMovement* cameraScript = nullptr;
    PerfectTimer cameraProgression;

    GameObject* playerGameObject = nullptr;
    Gate* gate = nullptr;

    C_AudioSource* clearedRoomAudio = nullptr;

    GameObject* groguGameObject = nullptr;
    const char* saveFileName = "GameState.json";
    int	currentLevel = 0;
    int	roomNum = 0;

    std::vector<std::pair<bool, Entity*>> enemies;

    bool move = false; // shhhhhh, don't tell jordi
    float3 spawnPoint = float3::zero;
    std::vector<float3> backtrack;
    float backtrackDuration = 1.0f;
    Timer backtrackTimer;

    bool instantiatedSandstorm = false;
   

    // Items
    void LoadItemPool(std::vector<ItemData*>& pool, std::string path);
    std::vector<ItemData*> chestItemPool;
    std::vector<ItemData*> shopItemPool;
    std::vector<ItemData*> hubItemPool;

    // Chest
    Entity* lastEnemyDead = nullptr;

public:
    //Armorer Item Lvls
    unsigned int armorLvl = 0;
    unsigned int bootsLvl = 0;
    unsigned int ticketLvl = 0;
    unsigned int bottleLvl = 0;

};

SCRIPTS_FUNCTION GameManager* CreateGameManager();