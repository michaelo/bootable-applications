#include "entity.h"

static const EntityId MAX_ENTITY_ID = POOL_SIZE; // Maximum number of entities
static EntityId nextEntityId = 1; // Global variable to track the next entity ID
static Entity entities[MAX_ENTITY_ID]; //Global array to hold all entities

Entity * createEntity(uint16_t * name, ComponentFlags components)
{
    EntityId id = nextEntityId++;
    if (id >= MAX_ENTITY_ID) {
        // Handle error: maximum number of entities reached
        return NULL;
    }
    Entity * entity = &entities[id]; 
    *entity = (Entity) {
        .id = id, 
        .components = components,
        .name = name 
    };

    return entity;
}
