#ifndef ENTITY_H
#define ENTITY_H

#include "inttypes.h"
#include "ecs.h"
#include "component.h"

typedef enum {
    ENTITY_ACTIVE = 1ULL << 0, // Entity is active
    ENTITY_VISIBLE = 1ULL << 1, // Entity is visible
    ENTITY_DELETED = 1ULL << 2, // Entity is marked for deletion
    ENTITY_LAST_POSSIBLE = 1ULL << 63 // Last possible flag value
} EntityStateFlags;

typedef struct {
    EntityId id; // Unique identifier for the entity
    ComponentFlags components; // Flags indicating which components this entity has
    uint16_t * name; //pointer to entity name, if any. Stored as a UTF-16 string
    uint64_t state; // State flags for the entity, e.g., active, visible, selected
    void * userData; // Pointer to user-defined data associated with the entity
} Entity;

Entity * createEntity(uint16_t * name, ComponentFlags components);

#endif // ENTITY_H
