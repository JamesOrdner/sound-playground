#include "EInput.h"
#include "Engine.h"
#include "EWorld.h"
#include "EObject.h"
#include "EInputComponent.h"

void EInput::handleInput(const SDL_Event& event)
{
    const auto& objects = Engine::instance().world().allObjects();
    for (auto& object : objects) {
        if (auto* inputComponent = object->inputComponent()) {
            inputComponent->processInput(event);
        }
    }
}
