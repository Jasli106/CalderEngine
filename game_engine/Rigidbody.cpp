//
//  Rigidbody.cpp
//  game_engine
//
//  Created by Jasmine Li on 3/31/24.
//

#include "Rigidbody.hpp"
#include "glm/glm.hpp"

void CollisionDetector::BeginContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
    Collision collision;
    collision.other = actorB;
    b2WorldManifold worldManifold;
    contact->GetWorldManifold(&worldManifold);
    collision.point = worldManifold.points[0];
    collision.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
    collision.normal = worldManifold.normal;
    if(fixtureA->IsSensor() && fixtureB->IsSensor()) { // Trigger
        collision.point = b2Vec2(-999.0f,-999.0f);
        collision.normal = b2Vec2(-999.0f,-999.0f);
        actorA->OnTriggerEnter(collision);
        collision.other = actorA;
        actorB->OnTriggerEnter(collision);
    } else if(!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
        actorA->OnCollisionEnter(collision);
        collision.other = actorA;
        actorB->OnCollisionEnter(collision);
    }
}

void CollisionDetector::EndContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    Actor* actorA = reinterpret_cast<Actor*>(fixtureA->GetUserData().pointer);
    Actor* actorB = reinterpret_cast<Actor*>(fixtureB->GetUserData().pointer);
    Collision collision;
    collision.other = actorB;
    collision.point = b2Vec2(-999.0f,-999.0f);
    collision.relative_velocity = fixtureA->GetBody()->GetLinearVelocity() - fixtureB->GetBody()->GetLinearVelocity();
    collision.normal = b2Vec2(-999.0f,-999.0f);
    if(fixtureA->IsSensor() && fixtureB->IsSensor()) { // Trigger
        actorA->OnTriggerExit(collision);
        collision.other = actorA;
        actorB->OnTriggerExit(collision);
    } else if(!fixtureA->IsSensor() && !fixtureB->IsSensor()) {
        actorA->OnCollisionExit(collision);
        collision.other = actorA;
        actorB->OnCollisionExit(collision);
    }
}

b2World* Physics::world = nullptr;
CollisionDetector* Physics::collisionDetector = nullptr;

float degToRad(float deg) {
    return deg * (b2_pi/180.0f);
}

float radToDeg(float rad) {
    return rad * (180.0f/b2_pi);
}

void Physics::Step() {
    if(Physics::world != nullptr) {
        Physics::world->Step((1.0f/60.0f), 8, 3);
    }
}

float RaycastFirstCallback::ReportFixture(b2Fixture* fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction) {
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
    if (actor == nullptr) {
        return -1.0f;
    }

    _hitFixture = fixture;
    _hitPoint = point;
    _hitNormal = normal;
    _fraction = fraction;

    return 0.0f;
}

float RaycastAllCallback::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) {
    Actor* actor = reinterpret_cast<Actor*>(fixture->GetUserData().pointer);
    if (actor == nullptr) {
        return -1.0f;
    }

    HitResult hit;
    hit.actor = actor;
    hit.point = point;
    hit.normal = normal;
    hit.is_trigger = fixture->IsSensor();
    hits.push_back(hit);
    return 1.0f;
}

HitResult Physics::Raycast(b2Vec2 pos, b2Vec2 dir, float dist) {
    if (dist <= 0 || !Physics::world) {
        return luabridge::LuaRef(ComponentDB::GetLuaState());
    }
    RaycastFirstCallback callback;
    dir.Normalize();
    Physics::world->RayCast(&callback, pos, pos + (dist*dir));
    if (callback._hitFixture != nullptr) {
        HitResult result;
        b2FixtureUserData userData = callback._hitFixture->GetUserData();
        result.actor = reinterpret_cast<Actor*>(callback._hitFixture->GetUserData().pointer);
        result.point = callback._hitPoint;
        result.normal = callback._hitNormal;
        result.is_trigger = callback._hitFixture->IsSensor();
        return result;
    }
    return luabridge::LuaRef(ComponentDB::GetLuaState());
}

luabridge::LuaRef Physics::RaycastAll (b2Vec2 pos, b2Vec2 dir, float dist) {
    if (dist <= 0 || !Physics::world) {
        return luabridge::LuaRef(ComponentDB::GetLuaState());
    }

    RaycastAllCallback callback;
    dir.Normalize();
    world->RayCast(&callback, pos, pos + (dist*dir));
    
    std::stable_sort(callback.hits.begin(), callback.hits.end(),
        [pos](const HitResult& a, const HitResult& b) {
            return (a.point-pos).LengthSquared() < (b.point-pos).LengthSquared();
        });
    
    luabridge::LuaRef hitResults = luabridge::newTable(ComponentDB::GetLuaState());

    int index = 1;
    for (const HitResult& hit : callback.hits) {
        hitResults[index++] = hit;
    }

    return hitResults;
}

void Rigidbody::Ready() {
    // Create b2World object
    if(!Physics::world) {
        Physics::world = new b2World(b2Vec2(0.0f, 9.8f));
        Physics::collisionDetector = new CollisionDetector();
        Physics::world->SetContactListener(Physics::collisionDetector);
    }
    
    // Create body
    b2BodyDef body_def;
    if(body_type == "dynamic") {
        body_def.type = b2_dynamicBody;
    }
    else if(body_type == "kinematic") {
        body_def.type = b2_kinematicBody;
    }
    else if(body_type == "static") {
        body_def.type = b2_staticBody;
    }
    body_def.position = b2Vec2(x, y);
    body_def.bullet = precise;
    body_def.gravityScale = gravity_scale;
    body_def.angularDamping = angular_friction;
    
    Rigidbody::body = Physics::world->CreateBody(&body_def);
    
    // Create fixture
    if(!has_trigger && !has_collider) {
        b2PolygonShape shape;
        shape.SetAsBox(width / 2.0f, height / 2.0f);
        
        b2FixtureDef fixture_def;
        fixture_def.shape = &shape;
        fixture_def.density = density;
        
        fixture_def.isSensor = true;
        fixture_def.filter.maskBits = 0x0000; 
        fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(nullptr);
        Rigidbody::body->CreateFixture(&fixture_def);
    }
    else {
        if(has_collider) {
            b2FixtureDef fixture_def;
            b2PolygonShape polygonShape;
            b2CircleShape circleShape;
            
            if(collider_type == "box") {
                polygonShape.SetAsBox(width / 2.0f, height / 2.0f);
                fixture_def.shape = &polygonShape;
            } else if(collider_type == "circle") {
                circleShape.m_radius = radius;
                fixture_def.shape = &circleShape;
            }
            
            fixture_def.density = density;
            fixture_def.isSensor = false;
            fixture_def.friction = friction;
            fixture_def.restitution = bounciness;
            fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            Rigidbody::body->CreateFixture(&fixture_def);
        }
        if (has_trigger) {
            b2FixtureDef fixture_def;
            b2PolygonShape polygonShape;
            b2CircleShape circleShape;
            
            if(trigger_type == "box") {
                polygonShape.SetAsBox(trigger_width / 2.0f, trigger_height / 2.0f);
                fixture_def.shape = &polygonShape;
            } else if(trigger_type == "circle") {
                circleShape.m_radius = trigger_radius;
                fixture_def.shape = &circleShape;
            }
            
            fixture_def.density = density;
            fixture_def.isSensor = true;
            fixture_def.userData.pointer = reinterpret_cast<uintptr_t>(actor);
            Rigidbody::body->CreateFixture(&fixture_def);
        }
    }
    
    SetRotation(rotation);
}

void Rigidbody::OnDestroy() {
    Physics::world->DestroyBody(body);
}

b2Vec2 Rigidbody::GetPosition() {
    return Rigidbody::body->GetPosition();
}

float Rigidbody::GetRotation() {
    return radToDeg(body->GetAngle());
}

void Rigidbody::AddForce(b2Vec2 force) {
    body->ApplyForceToCenter(force, true);
}

void Rigidbody::SetVelocity(b2Vec2 vel) {
    body->SetLinearVelocity(vel);
}

void Rigidbody::SetPosition(b2Vec2 pos) {
    body->SetTransform(pos, rotation);
}

void Rigidbody::SetRotation(float deg_clockwise) {
    body->SetTransform(body->GetPosition(), degToRad(deg_clockwise));
}

void Rigidbody::SetAngularVelocity(float deg_clockwise) {
    body->SetAngularVelocity(degToRad(deg_clockwise));
}

void Rigidbody::SetGravityScale(float scale) {
    body->SetGravityScale(scale);
}

void Rigidbody::SetUpDirection(b2Vec2 dir) {
    dir.Normalize();
    float new_angle_rads = glm::atan(dir.x, -dir.y);
    float new_angle_degs = radToDeg(new_angle_rads);
    SetRotation(new_angle_degs);
}

void Rigidbody::SetRightDirection(b2Vec2 dir) {
    dir.Normalize();
    float new_angle_rads = glm::atan(dir.x, -dir.y) - (b2_pi / 2.0f);
    float new_angle_degs = radToDeg(new_angle_rads);
    SetRotation(new_angle_degs);
}

b2Vec2 Rigidbody::GetVelocity() {
    return body->GetLinearVelocity();
}

float Rigidbody::GetAngularVelocity() {
    return radToDeg(body->GetAngularVelocity());
}

float Rigidbody::GetGravityScale() {
    return body->GetGravityScale();
}

b2Vec2 Rigidbody::GetUpDirection() {
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::sin(angle), -glm::cos(angle));
    result.Normalize();
    return result;
}

b2Vec2 Rigidbody::GetRightDirection() {
    float angle = body->GetAngle();
    b2Vec2 result = b2Vec2(glm::cos(angle), glm::sin(angle));
    result.Normalize();
    return result;
}
