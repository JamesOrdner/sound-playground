#include "EInput.h"
#include "Engine.h"
#include "EWorld.h"
#include "EObject.h"
#include "EInputComponent.h"

void EInput::handleInput(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_KEYDOWN:
        handleKeypress(event);
        break;
    }
}

void EInput::handleKeypress(const SDL_Event& event)
{
    SDL_Keycode keycode = event.key.keysym.sym;

    const auto& objects = Engine::instance().world().allObjects();
    for (auto& object : objects) {
        if (auto* inputComponent = object->inputComponent()) {
            inputComponent->processInput(keycode);
        }
    }
}
