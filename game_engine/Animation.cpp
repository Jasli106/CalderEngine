//
//  Animation.cpp
//  game_engine
//
//  Created by Jasmine Li on 4/15/24.
//

#include "Animation.hpp"
#include "SceneDB.hpp"

AnimAtlasFile::AnimAtlasFile(std::string initialFilePath) :
        AtlasFile(initialFilePath) {}
void AnimAtlasFile::renderSprite(SpriterEngine::UniversalObjectInterface *spriteInfo, const SpriterEngine::atlasframedata data) {}

AnimImageFile::AnimImageFile(std::string initialFilePath, SpriterEngine::point initialDefaultPivot)
    : ImageFile(initialFilePath, initialDefaultPivot) {
        std::filesystem::path image_name = initialFilePath;
        image_name.replace_extension("");
        if(Image::loaded_imgs.find(image_name) == Image::loaded_imgs.end()) {
            std::filesystem::path path = resources/"images"/(image_name.string() +".png");
            if(!std::filesystem::exists(path)) {
                std::cout << "error: missing image " + image_name.string();
                exit(0);
            }
            Image::loaded_imgs[image_name] = IMG_LoadTexture(Scene::renderer, path.string().c_str());
        }
        texture = Image::loaded_imgs[image_name];
}
void AnimImageFile::renderSprite(SpriterEngine::UniversalObjectInterface *spriteInfo) {

    SDL_Rect rect;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    
    float scale_x = spriteInfo->getScale().x;
    float scale_y = spriteInfo->getScale().y;
    
    float rotation = spriteInfo->getAngle() * (180/M_PI);
    
    glm::vec2 pos = glm::vec2(spriteInfo->getPosition().x, spriteInfo->getPosition().y);
    
    
    glm::vec2 cam_adj_pos = pos - Camera::camera_pos;

    rect.w *= std::abs(scale_x);
    rect.h *= std::abs(scale_y);
    
    SDL_Point* pivot = new SDL_Point({
        static_cast<int>(spriteInfo->getPivot().x * rect.w),
        static_cast<int>(spriteInfo->getPivot().y * rect.h)
    });

    rect.x = cam_adj_pos.x + Camera::resolution.x * 0.5f;
    rect.y = cam_adj_pos.y + Camera::resolution.y * 0.5f;

    Helper::SDL_RenderCopyEx498(0, "", Scene::renderer, texture, nullptr, &rect, rotation, pivot, SDL_FLIP_NONE);
}
void AnimImageFile::setAtlasFile(SpriterEngine::AtlasFile* initialAtlasFile, SpriterEngine::atlasframedata initialAtlasFrameData) {
    ImageFile::setAtlasFile(initialAtlasFile, initialAtlasFrameData);
}

// ---------------------------- //
std::string AnimSpriterFileAttributeWrapper::getName() {
    return m_name;
}
bool AnimSpriterFileAttributeWrapper::isValid() {
    return (m_value != nullptr && !m_value->IsNull());
}
SpriterEngine::real AnimSpriterFileAttributeWrapper::getRealValue() {
    if (m_value->IsNumber()) {
        return static_cast<SpriterEngine::real>(m_value->GetDouble());
    } else {
        return 0;
    }
}
int AnimSpriterFileAttributeWrapper::getIntValue() {
    if(m_value->IsString()) {
        std::string str = m_value->GetString();
        return std::stoi(str);
    }
    else if (m_value->IsInt()) {
        return m_value->GetInt();
    } else {
        return 0;
    }
}
std::string AnimSpriterFileAttributeWrapper::getStringValue() {
    if (m_value->IsString()) {
        return std::string(m_value->GetString());
    } else {
        return "";
    }
}
void AnimSpriterFileAttributeWrapper::advanceToNextAttribute() {
    m_parent_object->advanceToNextAttribute(this);
}

AnimSpriterFileElementWrapper::AnimSpriterFileElementWrapper(AnimSpriterFileElementWrapper* parent, std::string name, const rapidjson::Value* value, int index) :
m_parent(parent), m_name(name), m_value(value), m_index(index) {
}
std::string AnimSpriterFileElementWrapper::getName() {
    return m_name;
}
bool AnimSpriterFileElementWrapper::isValid() {
    if (m_value && m_value->IsArray()) {
        if(m_index >= m_value->GetArray().Size())
            return false;
        const rapidjson::Value& object_value = (*m_value)[m_index];
        if(!object_value.IsObject()) {
            return false;
        }
        return true;
    }
    return false;
}
void AnimSpriterFileElementWrapper::advanceToNextSiblingElement() {
    m_index++;
    
    if(isValid()) {
        return;
    }

    if(!m_parent) {
        return;
    }

    if(!m_parent->isValid())  {
        return;
    }

    const rapidjson::Value& object = (*m_parent->m_value)[(m_parent->m_index)];

    auto it = object.FindMember(m_name.c_str());
    if (it == object.MemberEnd()) {
        return;
    }
    ++it;
    if(it == object.MemberEnd()) {
        return;
    }
    for (; it != object.MemberEnd(); ++it) {
        if(!(it->value.IsObject() || it->value.IsArray())) {
            continue;
        }
        if(it->value.IsObject()) {
            m_array.SetArray();
            m_array.PushBack(rapidjson::Value(it->value, m_document.GetAllocator()), m_document.GetAllocator());
            m_value = &m_array;
            m_index = 0;
            m_name = it->name.GetString();
        }
        else if(it->value.IsArray()) {
            if (it->value.Empty()) { continue; }
            m_value = &it->value;
            m_index = 0;
            m_name = it->name.GetString();
        }
        return;
    }
}
void AnimSpriterFileElementWrapper::advanceToNextSiblingElementOfSameName() {
    ++m_index;
}
void AnimSpriterFileElementWrapper::advanceToNextAttribute(AnimSpriterFileAttributeWrapper* from_attribute) {
    if(!isValid()) {
        from_attribute->m_value = nullptr;
        from_attribute->m_name = "";
        return;
    }
    if(!from_attribute->isValid()) {
        from_attribute->m_value = nullptr;
        from_attribute->m_name = "";
        return;
    }
    const rapidjson::Value& object = (*m_value)[m_index];
    if (object.IsObject() && !object.ObjectEmpty()) {
        auto it = object.MemberBegin();
        for (; it != object.MemberEnd(); ++it) {
            if (std::strcmp(it->name.GetString(), from_attribute->m_name.c_str()) == 0) {
                ++it;
                break;
            }
        }

        if (it == object.MemberEnd()) {
            from_attribute->m_value = nullptr;
            from_attribute->m_name.clear();
            return;
        }

        while (it != object.MemberEnd() && (it->value.IsObject() || it->value.IsArray())) {
            ++it;
        }

        if (it != object.MemberEnd()) {
            from_attribute->m_value = &(it->value);
            from_attribute->m_name = it->name.GetString();
        } else {
            from_attribute->m_value = nullptr;
            from_attribute->m_name.clear();
        }
    } else {
        from_attribute->m_value = nullptr;
        from_attribute->m_name.clear();
    }

}
AnimSpriterFileAttributeWrapper* AnimSpriterFileElementWrapper::newAttributeWrapperFromFirstAttribute() {
    if(!isValid()) {
        return new AnimSpriterFileAttributeWrapper(this);
    }

    const rapidjson::Value& object = (*m_value)[m_index];

    for (auto it = object.MemberBegin(); it != object.MemberEnd(); ++it) {
        if (it->value.IsObject() || it->value.IsArray()) {
            continue;
        }
        return new AnimSpriterFileAttributeWrapper(this, it->name.GetString(), &(it->value));
    }
    return new AnimSpriterFileAttributeWrapper(this);
}
AnimSpriterFileAttributeWrapper* AnimSpriterFileElementWrapper::newAttributeWrapperFromFirstAttribute(const std::string & attributeName) {
    if(!isValid()) {
        return new AnimSpriterFileAttributeWrapper(this, attributeName);
    }
    const rapidjson::Value &object = (*m_value)[m_index];

    auto it = object.FindMember(attributeName.c_str());
    if (it == object.MemberEnd() || it->value.IsArray()||it->value.IsObject()) {
        return new AnimSpriterFileAttributeWrapper(this, attributeName);
    }
    return new AnimSpriterFileAttributeWrapper(this, attributeName, &(it->value));
}
SpriterEngine::SpriterFileElementWrapper *AnimSpriterFileElementWrapper::newElementWrapperFromFirstElement() {
    if (!isValid()) {
        return new AnimSpriterFileElementWrapper(this);
    }
    const rapidjson::Value& currentObject = (*m_value)[m_index];

    for (auto it = currentObject.MemberBegin(); it != currentObject.MemberEnd(); ++it) {
        if (!it->value.IsObject() && !it->value.IsArray()) {
            continue;
        }
        
        if (it->value.IsObject()) {
            m_array.SetArray();
            m_array.PushBack(rapidjson::Value(it->value, m_document.GetAllocator()), m_document.GetAllocator());
            return new AnimSpriterFileElementWrapper(this, it->name.GetString(), &m_array, 0);
        }
        
        if (it->value.IsArray() && !it->value.Empty()) {
            return new AnimSpriterFileElementWrapper(this, it->name.GetString(), &(it->value), 0);
        }
    }

    return new AnimSpriterFileElementWrapper(this);
}
SpriterEngine::SpriterFileElementWrapper* AnimSpriterFileElementWrapper::newElementWrapperFromFirstElement(const std::string & elementName) {
    if(!isValid()) {
        return new AnimSpriterFileElementWrapper(this, elementName);
    }
    const rapidjson::Value &object = (*m_value)[m_index];

    auto it = object.FindMember(elementName.c_str());
    if (it != object.MemberEnd()) {
        if (it->value.IsArray()) {
            return new AnimSpriterFileElementWrapper(this, elementName, &it->value, 0);
        }
        if (it->value.IsObject()) {
            m_array.SetArray();
            m_array.PushBack(rapidjson::Value(it->value, m_document.GetAllocator()), m_document.GetAllocator());
            AnimSpriterFileElementWrapper* newElemWrapper = new AnimSpriterFileElementWrapper(nullptr, elementName);
            newElemWrapper->m_value = &m_array;
            return newElemWrapper;
        }
    }
    return new AnimSpriterFileElementWrapper(this, elementName);
}
SpriterEngine::SpriterFileElementWrapper* AnimSpriterFileElementWrapper::newElementWrapperFromNextSiblingElement() {
    AnimSpriterFileElementWrapper *wrapper = new AnimSpriterFileElementWrapper(m_parent, m_name, m_value, m_index);
    wrapper->advanceToNextSiblingElement();
    return wrapper;
}
SpriterEngine::SpriterFileElementWrapper* AnimSpriterFileElementWrapper::newElementClone() {
    return new AnimSpriterFileElementWrapper(m_parent, m_name, m_value, m_index);
}

void AnimSpriterFileDocumentWrapper::loadFile(std::string fileName) {
    std::filesystem::path path = resources/"animations"/(fileName);
    if(!std::filesystem::exists(path)) {
        std::cout << "error: missing animation file " + fileName;
        exit(0);
    }
    EngineUtils::ReadJsonFile(path, doc);
}
SpriterEngine::SpriterFileElementWrapper* AnimSpriterFileDocumentWrapper::newElementWrapperFromFirstElement() {
    if (doc.IsObject() && !doc.ObjectEmpty()) {
        array.SetArray();
        array.PushBack(doc, doc.GetAllocator());
        const rapidjson::Value* val = &array;
        AnimSpriterFileElementWrapper* newElemWrapper = new AnimSpriterFileElementWrapper(nullptr);
        newElemWrapper->m_value = val;
        return newElemWrapper;
    }
    return new AnimSpriterFileElementWrapper(nullptr);
}
SpriterEngine::SpriterFileElementWrapper* AnimSpriterFileDocumentWrapper::newElementWrapperFromFirstElement(const std::string &elementName) {
    assert(elementName.compare("spriter_data") == 0);
    return newElementWrapperFromFirstElement();
}

// ---------------------------- //

AnimImageFile* AnimFileFactory::newImageFile(const std::string &initialFilePath, SpriterEngine::point initialDefaultPivot, SpriterEngine::atlasdata atlasData) {
    return new AnimImageFile(initialFilePath, initialDefaultPivot);
}
AnimSoundFile* AnimFileFactory::newSoundFile(const std::string &initialFilePath) {
    return new AnimSoundFile(initialFilePath);
}
AnimAtlasFile* AnimFileFactory::newAtlasFile(const std::string &initialFilePath) {
    return new AnimAtlasFile(initialFilePath);
}
AnimSpriterFileDocumentWrapper* AnimFileFactory::newSconDocumentWrapper(){
    return new AnimSpriterFileDocumentWrapper();
}

void AnimationComponent::Ready() {
    fileFactory = new AnimFileFactory();
    model = new SpriterEngine::SpriterModel(file, fileFactory);
    entityInstance = model->getNewEntityInstance(0);
}

void AnimationComponent::OnUpdate() {
    // Update internal time
    if(internalTime >= 0) {
        internalTime += deltaTime;
            
        if(entityInstance->animationJustFinished()) {
            internalTime = 0;
        }
        entityInstance->setTimeElapsed(deltaTime);
    }
}

void AnimationComponent::OnDestroy() {
}

void AnimationComponent::Play(std::string anim_name) {
    // Add to render queue
    AnimationDB::to_render[key] = entityInstance;
    entityInstance->setCurrentAnimation(anim_name);
    internalTime = 0;
}

void AnimationComponent::Stop() {
    // Remove from render queue
    AnimationDB::to_render.erase(AnimationDB::to_render.find(key));
    internalTime = -1;
}

void AnimationComponent::SetPosition(float x, float y) {
    position.x = x;
    position.y = y;
    const SpriterEngine::point newPos(x, y);
    entityInstance->setPosition(newPos);
}
void AnimationComponent::SetScale(float x, float y) {
    scale.x = x;
    scale.y = y;
    const SpriterEngine::point newScale(x, y);
    entityInstance->setScale(newScale);
}
void AnimationComponent::SetRotation(float rot) {
    rotation = rot;
    entityInstance->setAngle(SpriterEngine::toRadians(rot));
}
glm::vec2 AnimationComponent::GetPosition() {
    return position;
}
glm::vec2 AnimationComponent::GetScale() {
    return scale;
}
float AnimationComponent::GetRotation() {
    return rotation;
}
