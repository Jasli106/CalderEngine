//
//  Rigidbody.hpp
//  game_engine
//
//  Created by Jasmine Li on 3/31/24.
//

#ifndef Rigidbody_hpp
#define Rigidbody_hpp

#include <stdio.h>
#include <string>
#include <box2d/box2d.h>
#include "Actor.hpp"

struct Collision {
    Actor* other;
    b2Vec2 point;
    b2Vec2 relative_velocity;
    b2Vec2 normal;
};

class CollisionDetector : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

struct HitResult {
    Actor* actor;
    b2Vec2 point;
    b2Vec2 normal;
    bool is_trigger;
};

class RaycastFirstCallback : public b2RayCastCallback {
public:
    b2Fixture* _hitFixture;
    b2Vec2 _hitPoint;
    b2Vec2 _hitNormal;
    float _fraction;
    float ReportFixture(b2Fixture* fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction) override;
};

class RaycastAllCallback : public b2RayCastCallback {
public:
    std::vector<HitResult> hits;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override;
};

class Physics {
public:
    static b2World* world;
    static CollisionDetector* collisionDetector;
    static void Step();
    
    static HitResult Raycast(b2Vec2 pos, b2Vec2 dir, float dist);
    static luabridge::LuaRef RaycastAll (b2Vec2 pos, b2Vec2 dir, float dist);
};

class Rigidbody {
public:
    std::string type = "Rigidbody";
    std::string key = "???";
    Actor* actor = nullptr;
    bool enabled = true;
    
    float x = 0;
    float y = 0;
    std::string body_type = "dynamic";
    bool precise = true;
    float gravity_scale = 1.0f;
    float density = 1.0f;
    float angular_friction = 0.3f;
    float rotation = 0.0f;
    
    bool has_collider = true;
    std::string collider_type = "box";
    float width = 1.0f;
    float height = 1.0f;
    float radius = 0.5f;
    float friction = 0.3f;
    float bounciness = 0.3f;

    bool has_trigger = true;
    std::string trigger_type = "box";
    float trigger_width = 1.0f;
    float trigger_height = 1.0f;
    float trigger_radius = 0.5f;
    
    void Ready();
    void OnDestroy();
    
    b2Vec2 GetPosition();
    float GetRotation();
    void AddForce(b2Vec2 force);
    void SetVelocity(b2Vec2 vel);
    void SetPosition(b2Vec2 pos);
    void SetRotation(float deg_clockwise);
    void SetAngularVelocity(float deg_clockwise);
    void SetGravityScale(float scale);
    void SetUpDirection(b2Vec2 dir);
    void SetRightDirection(b2Vec2 dir);
    b2Vec2 GetVelocity();
    float GetAngularVelocity();
    float GetGravityScale();
    b2Vec2 GetUpDirection();
    b2Vec2 GetRightDirection();
    
private:
    b2Body* body;
};

#endif /* Rigidbody_hpp */
