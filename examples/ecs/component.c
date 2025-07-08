#include "component.h" 
#include "ecs.h"
#include "entity.h"
#include "shared/memory.h"

#include <stddef.h>

void initializeComponents(ECS * ecs)
{
    ecs->components = (ComponentData) {
        .positions =  (Position *) malloc_page_aligned(POOL_SIZE * sizeof(Position)),
        .rotations =  (Rotation *) malloc_page_aligned(POOL_SIZE * sizeof(Rotation)),
        .scales =     (Scale *)    malloc_page_aligned(POOL_SIZE * sizeof(Scale)),
        .velocities = (Velocity *) malloc_page_aligned(POOL_SIZE * sizeof(Velocity))
    };
}
