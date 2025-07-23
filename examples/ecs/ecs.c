#include "ecs.h"
#include "component.h"
#include "entity.h"

void initializeEcs(ECS * ecs)
{
    initializeEntities(ecs);
    initializeComponents(ecs);
}