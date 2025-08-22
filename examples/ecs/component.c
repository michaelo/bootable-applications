#include "component.h" 
#include "ecs.h"
#include "entity.h"
#include "shared/memory.h"

#include <stddef.h>

#define ALLOC_POOL(type) (((type*)malloc_page_aligned(POOL_SIZE * sizeof(type))))

void initializeComponents(ECS * ecs)
{
    ecs->componentData = (ComponentData) {
        .parent       = ALLOC_POOL(Parent),
        .children     = ALLOC_POOL(Children),
        .position     = ALLOC_POOL(Position),
        .rotation     = ALLOC_POOL(Rotation),
        .scale        = ALLOC_POOL(Scale),
        .localToWorld = ALLOC_POOL(LocalToWorld),
        .velocity     = ALLOC_POOL(Velocity),
        .rigidBody    = ALLOC_POOL(RigidBody)
    };
}
