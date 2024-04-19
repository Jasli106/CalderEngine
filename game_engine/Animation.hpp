//
//  Animation.hpp
//  game_engine
//
//  Created by Jasmine Li on 4/15/24.
//

#ifndef Animation_hpp
#define Animation_hpp

#include <stdio.h>
#include "spriterengine.h"
#include "spriterengine/override/filefactory.h"
#include "spriterengine/override/imagefile.h"
#include "spriterengine/override/soundfile.h"
#include "spriterengine/override/atlasfile.h"
#include "spriterengine/override/spriterfileattributewrapper.h"
#include "spriterengine/override/spriterfileelementwrapper.h"
#include "spriterengine/override/spriterfiledocumentwrapper.h"
#include "Actor.hpp"
#include "ImageDB.hpp"

inline float deltaTime = 1000.0f/60;
class AnimImageFile : public SpriterEngine::ImageFile {
public:
    AnimImageFile(std::string initialFilePath, SpriterEngine::point initialDefaultPivot);
    void renderSprite(SpriterEngine::UniversalObjectInterface *spriteInfo) override;
    void setAtlasFile(SpriterEngine::AtlasFile* initialAtlasFile, SpriterEngine::atlasframedata initialAtlasFrameData) override;
private:
    SDL_Texture* texture;
};

class AnimSoundFile : public SpriterEngine::SoundFile {
public:
    AnimSoundFile(const std::string &initialFilePath)
        : SpriterEngine::SoundFile(initialFilePath) {}
    
//    AnimSoundObjectInfoReference *newSoundInfoReference() override;
};

class AnimAtlasFile : public SpriterEngine::AtlasFile {
public:
    AnimAtlasFile(std::string initialFilePath);
private:
    void renderSprite(SpriterEngine::UniversalObjectInterface *spriteInfo, const SpriterEngine::atlasframedata data) override;
};

class AnimSpriterFileElementWrapper;
class AnimSpriterFileAttributeWrapper : public SpriterEngine::SpriterFileAttributeWrapper
    {
    public:
        AnimSpriterFileAttributeWrapper(AnimSpriterFileElementWrapper* parent_object, std::string name = "", const rapidjson::Value* value = nullptr) : m_parent_object(parent_object), m_name(name), m_value(value) {}
        std::string getName() override;
        bool isValid() override;
        SpriterEngine::real getRealValue() override;
        int getIntValue() override;
        std::string getStringValue() override;
        void advanceToNextAttribute() override;

        std::string m_name;
        const rapidjson::Value* m_value;
        AnimSpriterFileElementWrapper* m_parent_object;
    };

class AnimSpriterFileElementWrapper : public SpriterEngine::SpriterFileElementWrapper {
public:
    AnimSpriterFileElementWrapper(AnimSpriterFileElementWrapper *parent = nullptr, std::string name = "", const rapidjson::Value* value = nullptr, int index = 0);
    AnimSpriterFileElementWrapper() {}
    std::string getName() override;
    bool isValid() override;
    void advanceToNextSiblingElement() override;
    void advanceToNextSiblingElementOfSameName() override;
    
    void advanceToNextAttribute(AnimSpriterFileAttributeWrapper* from_attribute);
    
    std::string m_name;
    const rapidjson::Value* m_value;
    rapidjson::Document m_document;
    rapidjson::Value m_array;
    int m_index;
    AnimSpriterFileElementWrapper* m_parent;
private:
    AnimSpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute() override;
    AnimSpriterFileAttributeWrapper *newAttributeWrapperFromFirstAttribute(const std::string & attributeName) override;
    SpriterFileElementWrapper *newElementWrapperFromFirstElement() override;
    SpriterFileElementWrapper *newElementWrapperFromFirstElement(const std::string & elementName) override;
    SpriterFileElementWrapper *newElementWrapperFromNextSiblingElement() override;
    SpriterFileElementWrapper *newElementClone() override;
    
};

class AnimSpriterFileDocumentWrapper : public SpriterEngine::SpriterFileDocumentWrapper {
public:
    AnimSpriterFileDocumentWrapper() {}
    void loadFile(std::string fileName) override;
protected:
    SpriterEngine::SpriterFileElementWrapper *newElementWrapperFromFirstElement() override;
    SpriterEngine::SpriterFileElementWrapper *newElementWrapperFromFirstElement(const std::string &elementName) override;
    rapidjson::Document doc;
    rapidjson::Value array;
};

class AnimFileFactory : public SpriterEngine::FileFactory {
public:
    AnimImageFile* newImageFile(const std::string &initialFilePath, SpriterEngine::point initialDefaultPivot, SpriterEngine::atlasdata atlasData) override;
    AnimSoundFile* newSoundFile(const std::string &initialFilePath) override;
    AnimAtlasFile* newAtlasFile(const std::string &initialFilePath) override;
    AnimSpriterFileDocumentWrapper* newSconDocumentWrapper() override;
};

class AnimationDrawRequest : public IDrawRequest {
public:
    SpriterEngine::EntityInstance* entityInstance;

    virtual void execute() override {
        entityInstance->render();
    }
    
    AnimationDrawRequest(int layer_type, int sorting_order, SpriterEngine::EntityInstance* entityInstance) : IDrawRequest(layer_type, sorting_order), entityInstance(entityInstance) {}
};

class AnimationComponent {
public:
    std::string type = "Animation";
    std::string key = "???";
    Actor* actor = nullptr;
    bool enabled = true;
    
    glm::vec2 position = glm::vec2(0,0);
    glm::vec2 scale = glm::vec2(1,1);
    float rotation = 0;
    
    std::string file = "";
    
    void Ready();
    void OnUpdate();
    void OnDestroy();
    
    void Play(std::string anim_name);
    void Stop();
    
    void SetPosition(float x, float y);
    void SetScale(float x, float y);
    void SetRotation(float rot);
    glm::vec2 GetPosition();
    glm::vec2 GetScale();
    float GetRotation();
    
private:
    float internalTime = -1;
    AnimFileFactory* fileFactory;
    SpriterEngine::SpriterModel* model;
    SpriterEngine::EntityInstance* entityInstance;
    
    std::unordered_set<std::string> animation_names;
};

class AnimationDB {
public:
    static inline std::map<std::string, SpriterEngine::EntityInstance*> to_render;
};

#endif /* Animation_hpp */
