#ifndef COMPONENT_H
#define COMPONENT_H

#define NUM_COMPONENTS 4 // Number of components implemented
#define MAX_COMPONENTS 64 // Number of components supported

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
    float x; // X coordinate
    float y; // Y coordinate
    float z; // Z coordinate (optional, for 3D)
    float w; // W coordinate (optional, for 4D or homogeneous coordinates)
} Position;

typedef struct {
    //FIXME: replace with quaternion representation?
    float pitch; // Pitch rotation (in degrees or radians)
    float yaw; // Yaw rotation (in degrees or radians)
    float roll; // Roll rotation (in degrees or radians)
    float angle; // Angle of rotation (optional, for quaternion representation)
} Rotation;

typedef struct {
    float sx; // Scale in X direction
    float sy; // Scale in Y direction
    float sz; // Scale in Z direction (optional, for 3D)
    float sw; // Scale in W direction (optional, for 4D or homogeneous coordinates)
} Scale;

typedef struct {
    float vx; // Velocity in X direction
    float vy; // Velocity in Y direction
    float vz; // Velocity in Z direction (optional, for 3D)
    float vw; // Velocity in W direction (optional, for 4D or homogeneous coordinates)
} Velocity;

typedef struct {
    Position * positions; 
    Rotation * rotations;
    Scale * scales;
    Velocity * velocities;
} ComponentData;

static ComponentData all_components;

void initializeComponents();
#endif // COMPONENT_H