#ifndef ENTITY_H
#define ENTITY_H

#include "inttypes.h"
#include "ecs.h"

typedef uint64_t EntityId; // Type for entity IDs
typedef uint64_t ComponentFlags; // Type for component IDs

typedef enum {
    ENTITY_ALIVE         = 1ULL << 0, // Entity is alive (not deleted)
    ENTITY_ACTIVE        = 1ULL << 1, // Entity is active (receives updates)
    ENTITY_VISIBLE       = 1ULL << 2, // Entity is visible
    ENTITY_DELETED       = 1ULL << 3, // Entity is marked for deletion
    ENTITY_LAST_POSSIBLE = 1ULL << 63 // Last possible flag value
} EntityStateFlags;

typedef struct {
    EntityId id; // Unique identifier for the entity
    ComponentFlags components; // Flags indicating which components this entity has
    uint16_t * name; //pointer to entity name, if any. Stored as a UTF-16 string
    uint64_t state; // State flags for the entity, e.g., active, visible, selected
    void * userData; // Pointer to user-defined data associated with the entity
} Entity;

typedef struct {
    Entity * entities; // Array of all entities
    EntityId nextEntityId;
} EntityData;

void initializeEntities(ECS * ecs);
Entity * createEntity(ECS * ecs, uint16_t * name, ComponentFlags components);

#endif // ENTITY_H
