#include "UIObject.h"

UIObject::UIObject() :
	anchor(UIAnchor::Center),
	scale(1.f),
	bAcceptsInput(false)
{
	value.floatVal = 0.f;
}
