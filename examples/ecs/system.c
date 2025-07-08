#include "system.h"
#include "ecs.h"

void parentSystem(ECS * ecs)
{
    //update children for entities that have a parent component pointing to them
    for (EntityId i = 0; i < ecs->entities.nextEntityId; i++)
    {        
        Entity *entity = &ecs->entities.entities[i];
        if (entity->components & COMPONENT_PARENT)
        {            
            // Get parent component
            Parent * parent = ecs->components.parent[i];
            if (parent)
            { 
                
            
            }
        }
    }

}