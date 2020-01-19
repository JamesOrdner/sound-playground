#include "UObject.h"

UObject::UObject()
{
}

void UObject::forwardEventImmediate(EventType event, const EventData& data) const
{
	for (const auto* object : subobjects) object->eventImmediate(event, data, true);
}
