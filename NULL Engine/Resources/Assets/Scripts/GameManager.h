#pragma once
#include "Script.h"
#include "ScriptMacros.h"
#include "LevelGenerator.h"

class SCRIPTS_API GameManager : public Script {
public:
    GameManager();
    ~GameManager();
    void Awake() override;
    void Update()override;

    void GenerateNewRun();
    LevelGenerator level;
};

SCRIPTS_FUNCTION GameManager* CreateGameManager();