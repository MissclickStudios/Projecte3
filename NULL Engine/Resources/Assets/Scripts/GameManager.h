#pragma once
#include "Script.h"
#include "ScriptMacros.h"

#include "Prefab.h"
#include "Timer.h"

#include "Player.h"

#define BACKTRACK 5

class GameObject;
class Gate;

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
    std::vector<std::string> level2;

    bool enabled;
    std::string mainMenuScene;
    std::string SpawnPointName;
    std::string gateName = "NextSceneTrigger";

    Prefab playerPrefab;
    Player* playerScript = nullptr;

private:
    GameObject* playerGameObject = nullptr;
    Gate* gate = nullptr;

    const char* saveFileName = "GameState.json";
    int	currentLevel = 0;
    int	roomNum = 0;

    std::vector<Entity*> enemies;

    bool move = false; // shhhhhh, don't tell jordi
    float3 spawnPoint = float3::zero;
    std::vector<float3> backtrack;
    float backtrackDuration = 1.0f;
    Timer backtrackTimer;
};

SCRIPTS_FUNCTION GameManager* CreateGameManager();