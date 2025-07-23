#ifndef ECS_H
#define ECS_H

#include "inttypes.h"
#include "component.h"
#include "entity.h"
#include "system.h"

static const int PAGE_SIZE = 4096; // Assuming a page size of 4096 bytes (what we get from the UEFI allocator))
static const int POOL_SIZE = 4096; // Number of instances of entities / instances per component type to allocate for

typedef struct ECS {
    EntityData entityData; // Array of all entities    
    ComponentData componentData; // All components, indexed by component type
} ECS;

void initializeEcs(ECS * ecs);
void updateEcs(ECS * ecs);

#endif // ECS_H
