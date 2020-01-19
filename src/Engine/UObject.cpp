#include "UObject.h"

UObject::UObject()
{
}

void UObject::childEventImmediate(EventType event, const EventData& data) const
{
	for (const auto* object : subobjects) object->eventImmediate(event, data, true);
}
