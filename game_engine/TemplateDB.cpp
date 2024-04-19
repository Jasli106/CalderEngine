#include "TemplateDB.h"
#include "ImageDB.hpp"

std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> templates;

Actor LoadActorFromTemplate(std::string name) {
    std::string path = (resources / "actor_templates" / (name + ".template")).generic_string();
	if (!std::filesystem::exists(path)) {
        std::cout << "error: template " << name << " is missing";
        exit(0);
    }
	if (templates.find(name) == templates.end()) { // New template
		std::unique_ptr<rapidjson::Document> temp = std::make_unique<rapidjson::Document>();
		EngineUtils::ReadJsonFile(path, *temp);
        
		Actor new_actor = CreateActor(*temp);
        templates[name] = std::move(temp);
		return new_actor;
	}
	else {
		return CreateActor(*templates[name]);
	}
}
