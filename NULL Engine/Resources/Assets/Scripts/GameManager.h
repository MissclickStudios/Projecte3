#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "Prefab.h"
#include "Player.h"

class GameObject;

class SCRIPTS_API GameManager : public Script {
public:
    GameManager();
    ~GameManager();
    void Awake() override;
    void Start()override;
    void Update()override;

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

public:
    std::vector<std::string> level1;
    std::vector<std::string> level2;

    bool enabled;
    std::string mainMenuScene;
    std::string SpawnPointName;
    Prefab playerPrefab;
    Player* playerScript = nullptr;

private:
    GameObject* playerGameObject = nullptr;
    const char* saveFileName = "GameState.json";
    int	currentLevel = 0;
    int	roomNum = 0;
};

SCRIPTS_FUNCTION GameManager* CreateGameManager();