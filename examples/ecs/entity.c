#include "entity.h"
#include "shared/memory.h"
#include "ecs.h"
#include "shared/efi_global_state.h"

static const EntityId MAX_ENTITY_ID = POOL_SIZE; // Maximum number of entities

void initializeEntities(ECS * ecs)
{
    ecs->entityData = (EntityData) {
        .entities = (Entity *) malloc_page_aligned(POOL_SIZE * sizeof(Entity)),
        .nextEntityId = 1
    };
    uefi_println(L"Initialized EntityData with %d entities.", POOL_SIZE);
}

Entity *createEntity(ECS * ecs, uint16_t *name, ComponentFlags components, EntityStateFlags stateFlags)
{
    EntityId id = ecs->entityData.nextEntityId++;
    if (id >= MAX_ENTITY_ID) {
        // Handle error: maximum number of entities reached
        return NULL;
    }
    Entity * entity = &ecs->entityData.entities[id]; 
    *entity = (Entity) {
        .id = id, 
        .components = components,
        .name = name,
        .state = stateFlags
    };

    return entity;
}
