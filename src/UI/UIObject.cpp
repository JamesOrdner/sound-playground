#include "UIObject.h"

UIObject::UIObject() :
	anchor(UIAnchor::Center),
	scale(1.f),
	bAcceptsInput(false)
{
	value.floatVal = 0.f;
}

mat::vec2 UIObject::anchorPosition() const
{
	using namespace mat;
	switch (anchor) {
	case UIAnchor::Center: return vec2{ 0.f, 0.f };
	case UIAnchor::Top: return vec2{ 0.f, 1.f };
	case UIAnchor::Bottom: return vec2{ 0.f, -1.f };
	case UIAnchor::Left: return vec2{ -1.f, 0.f };
	case UIAnchor::Right: return vec2{ 1.f, 0.f };
	case UIAnchor::TopLeft: return vec2{ -1.f, 1.f };
	case UIAnchor::TopRight: return vec2{ 1.f, 1.f };
	case UIAnchor::BottomLeft: return vec2{ -1.f, -1.f };
	case UIAnchor::BottomRight: return vec2{ 1.f, -1.f };
	default: return vec2();
	}
}
