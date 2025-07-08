The main idea is to make a super simple ECS. Based on some articles, it seems like the easiest implementation would be along the lines of:

- Store all components in separate arrays per component type
- An entity is simply an ID (unsigned int or similar) as well as a bitmask of its (active) components
- Components are stored in sparse arrays where the entity ID is the index of its components in their respective arrays

Eg. if we have the following component types (flags):

typedef enum {
    COMPONENT_POSITION = 1 << 0,
    COMPONENT_ROTATION = 1 << 1,
    COMPONENT_SCALE    = 1 << 2,
    COMPONENT_VELOCITY = 1 << 3
} Components;

We would have something like:

typedef struct 
{
    Position positions[];
    Rotation rotations[];
    Scale scales[];
    Velocity velocities[];
} Components;

typedef uint64_t EntityId;
typedef uint64_t Components; //bitmask

typedef struct
{
    EntityId id;
    Components components; 
} Entity;

Then we could create 2 entities: 

Entity 0 has position and scale:

Entity entity = {
    .id = 0,
    .components = COMPONENT_POSITION | COMPONENT_SCALE
}

Entity 1 has rotation and velocity:

Entity entity = {
    .id = 1,
    .components = COMPONENT_ROTATION | COMPONENT_VELOCITY
}

The Positions array would then hold entity 0's position in index 0, and a blank (unused) position in index 1.
Similarly, the rotation array would hold an unused (blank) rotation in index 0, and entity 1's rotation in index 1
