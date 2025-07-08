#ifndef COMPONENT_H
#define COMPONENT_H

#define NUM_COMPONENTS 4
#define MAX_COMPONENTS 64

typedef enum {
    COMPONENT_PARENT         = 1ULL << 0,
    COMPONENT_CHILDREN       = 1ULL << 1,
    COMPONENT_TRANSLATION    = 1ULL << 2,
    COMPONENT_ROTATION       = 1ULL << 3,
    COMPONENT_SCALE          = 1ULL << 4,
    COMPONENT_LOCAL_TO_WORLD = 1ULL << 5,
    COMPONENT_VELOCITY       = 1ULL << 5,
    COMPONENT_ACCELERATION   = 1ULL << 6,
    COMPONENT_LAST_POSSIBLE  = 1ULL << 63
} ComponentType;

typedef struct {
    EntityId parentEntity;
} Parent;

typedef struct {
    EntityId * children; // Array of child entity IDs
    size_t count; // Number of children
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
    float vx;
    float vy;
    float vz;
    float vw;
} Velocity;

typedef struct {
    Parent * parent;
    Children * children;
    Position * position;
    Rotation * rotation;
    Scale * scale;
    Velocity * velocity;
} ComponentData;

void initializeComponents(ECS * ecs);
#endif // COMPONENT_H