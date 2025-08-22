#ifndef COMPONENT_H
#define COMPONENT_H

#include "ecs_base.h"
#include "linkedlist.h"

#define NUM_COMPONENTS 4
#define MAX_COMPONENTS 64

typedef enum {
    COMPONENT_PARENT         = 1ULL << 0,
    COMPONENT_CHILDREN       = 1ULL << 1,
    COMPONENT_POSITION       = 1ULL << 2,
    COMPONENT_ROTATION       = 1ULL << 3,
    COMPONENT_SCALE          = 1ULL << 4,
    COMPONENT_LOCAL_TO_WORLD = 1ULL << 5,
    COMPONENT_VELOCITY       = 1ULL << 6,
    COMPONENT_RIGIDBODY      = 1ULL << 7,
    // COMPONENT_LAST           = 1ULL << 8,
    // COMPONENT_LAST_POSSIBLE  = 1ULL << 63
} ComponentType;

typedef struct {
    EntityId parent;
    EntityId previousParent;
} Parent;

typedef struct {
    List childrenList;
} Children;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Position;

typedef struct {
    float pitch;
    float yaw;
    float roll;
    float angle;
} Rotation;

typedef struct {
    float sx;
    float sy;
    float sz;
    float sw;
} Scale;

typedef struct {
    float matrix[4][4];
} LocalToWorld;

typedef struct {
    float vx;
    float vy;
    float vz;
    float vw;
} Velocity;

typedef enum {
    PHYSICS_USE_GRAVITY =  1U << 0,
    PHYSICS_IS_KINEMATIC = 1U << 1,
    PHYSICS_FREEZE_X =     1U << 2,
    PHYSICS_FREEZE_Y =     1U << 3,
    PHYSICS_FREEZE_Z =     1U << 4,
    PHYSICS_FREEZE_RX =    1U << 5,
    PHYSICS_FREEZE_RY =    1U << 6,
    PHYSICS_FREEZE_RZ =    1U << 7,
} PHYSICS_FLAGS;

typedef struct {
    float mass;
    float linearDamping;
    float angularDamping;
    uint32_t physicsFlags;
} RigidBody;

typedef struct {
    Parent * parent;
    Children * children;
    Position * position;
    Rotation * rotation;
    Scale * scale;
    LocalToWorld * localToWorld;
    Velocity * velocity;
    RigidBody * rigidBody; 
} ComponentData;

void initializeComponents(ECS * ecs);
#endif // COMPONENT_H