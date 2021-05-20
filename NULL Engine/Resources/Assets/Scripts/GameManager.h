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
class DialogManager;

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

    //Dialog

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
    DialogManager* dialogManager = nullptr;

private:
    GameObject* playerGameObject = nullptr;
    Gate* gate = nullptr;

    GameObject* groguGameObject = nullptr;
    const char* saveFileName = "GameState.json";
    int	currentLevel = 0;
    int	roomNum = 0;

    std::vector<Entity*> enemies;

    bool move = false; // shhhhhh, don't tell jordi
    float3 spawnPoint = float3::zero;
    std::vector<float3> backtrack;
    float backtrackDuration = 1.0f;
    Timer backtrackTimer;

    //Story vars
    bool visitedHUB = false;            //When mando enters the HUB for the first time a cinematic + dialog should happen
    bool defeatedIG11FirstTime = false; //When Mando hasn't defeated IG-11 Grogu should not be with him and IG-11 should use a special dialog
};

SCRIPTS_FUNCTION GameManager* CreateGameManager();