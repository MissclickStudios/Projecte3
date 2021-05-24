#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"
#include "Timer.h"

#include "Player.h"
#include "Grogu.h"

#define BACKTRACK 5

class GameObject;
class Gate;
class CameraMovement;
class DialogManager;

class PerfectTimer;

struct ItemData;

class StoryDialogData
{
public:
    bool visitedHUB = false;            //When mando enters the HUB for the first time a cinematic + dialog should happen
    bool defeatedIG11FirstTime = false; //When Mando hasn't defeated IG-11 Grogu should not be with him and IG-11 should use a special dialog
    bool defeatedIG12FirstTime = false;
    bool talkedToArmorer = false;
    bool firstTimeHub = false;
    bool talkedToGrogu = false;

    void Save(ParsonNode &node);
    void Load(ParsonNode &node);
    
};


class SCRIPTS_API GameManager : public Script 
{
public:

    GameManager();
    ~GameManager();
    void Awake() override;
    void Start()override;
    void Update()override; 

    void OnCollisionEnter(GameObject* object) override;

    void GenerateNewRun(bool fromMenu);
    void GoNextRoom();
    void InitiateLevel(int level);
    void Continue();
    void ReturnHub();
    void ReturnToMainMenu();

    std::vector<ItemData*> GetChestItemPool() const { return chestItemPool; };

    //Dialog & Story funtions
    void KilledIG11();
    void TalkedToArmorer();
    void BoughtFromArmorer();

private:
    //Level Generator
    void GenerateLevel();
    void GoPreviousRoom();
    void AddFixedRoom(std::string name, int level, int position);
    void HandleRoomGeneration();

    void SaveManagerState();

    void BackTrackUpdate();
    void GateUpdate();

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

    
    
    Prefab mistPlane1;
    Prefab mistPlane2;
    float3 mistPlane1Position = float3::zero;
    float3 mistPlane2Position = float3::zero;

    DialogManager* dialogManager = nullptr;


    Prefab chestPrefab;
    int chestSpawnChance = 60;

    std::string cameraName = "GameCameraVS2";

private:

    GameObject* cameraGameObject = nullptr;
    CameraMovement* cameraScript = nullptr;
    PerfectTimer cameraProgression;

    GameObject* playerGameObject = nullptr;
    Gate* gate = nullptr;

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

    //Story & dialog vars
    StoryDialogData storyDialogState;

    // Items
    std::vector<ItemData*> chestItemPool;

    // Chest
    Entity* lastEnemyDead = nullptr;

};

SCRIPTS_FUNCTION GameManager* CreateGameManager();