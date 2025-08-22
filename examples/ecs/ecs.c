#include "ecs.h"
#include "component.h"
#include "entity.h"
#include "shared/efi_global_state.h"

void initializeEcs(ECS * ecs)
{
    uefi_println(L"Initializing ECS at %x", ecs);

    initializeEntities(ecs);
    initializeComponents(ecs);
}