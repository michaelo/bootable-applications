#include "entity.h"
#include "shared/memory.h"
#include "ecs.h"

static const EntityId MAX_ENTITY_ID = POOL_SIZE; // Maximum number of entities

void initializeEntities(ECS * ecs)
{
    ecs->entities = (EntityData) {
        .entities = (Entity *) malloc_page_aligned(MAX_ENTITY_ID * sizeof(Entity)),
        .nextEntityId = 0
    };
}

Entity *createEntity(ECS * ecs, uint16_t *name, ComponentFlags components)
{
    EntityId id = ecs->entities->nextEntityId++;
    if (id >= MAX_ENTITY_ID) {
        // Handle error: maximum number of entities reached
        return NULL;
    }
    Entity * entity = &ecs->entities[id]; 
    *entity = (Entity) {
        .id = id, 
        .components = components,
        .name = name,
        .state = ENTITY_ALIVE | ENTITY_ACTIVE | ENTITY_VISIBLE, // Default state flags
    };

    return entity;
}
