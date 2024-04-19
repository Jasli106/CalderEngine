#pragma once

#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include "SceneDB.hpp"
#include "rapidjson/document.h"
#include "EngineUtils.h"

extern std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> templates;

Actor LoadActorFromTemplate(std::string name);
