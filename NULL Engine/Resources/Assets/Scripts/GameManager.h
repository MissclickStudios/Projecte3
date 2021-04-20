#pragma once
#include "Script.h"
#include "ScriptMacros.h"

class SCRIPTS_API GameManager : public Script {
public:
    GameManager();
    ~GameManager();
    void Awake() override;
    void Update()override;

    void GenerateNewRun();
    void GoNextRoom();
    void InitiateLevel(int level);
    void Continue();

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

private:
    const char* saveFileName = "GameState.json";
    int	currentLevel = 0;
    int	roomNum = 0;
};

SCRIPTS_FUNCTION GameManager* CreateGameManager();