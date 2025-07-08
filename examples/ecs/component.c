#include "component.h" 
#include "entity.h"
#include "shared/memory.h"

#include <stddef.h>

void initializeComponents()
{
    all_components = (ComponentData) {
        .positions =  (Position *) malloc(POOL_SIZE * sizeof(Position)),
        .rotations =  (Rotation *) malloc(POOL_SIZE * sizeof(Rotation)),
        .scales =     (Scale *)    malloc(POOL_SIZE * sizeof(Scale)),
        .velocities = (Velocity *) malloc(POOL_SIZE * sizeof(Velocity))
    };
}
