#include "UIObject.h"

UIObject::UIObject() :
	anchor(UIAnchor::Center),
	bAcceptsInput(false),
	state(UIObjectState::Neutral),
	animationRate(0.f),
	userData(nullptr)
{
	textureCoords = []() { return mat::vec4(); };
}

void UIObject::setAnimationTarget(const mat::vec2& position)
{
	animationTarget = position;
	if (animationRate == 0.f) this->position = position;
}

mat::vec2 UIObject::anchorPosition() const
{
	using mat::vec2;
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

void UIObject::tick(float deltaTime)
{
	for (auto& object : subobjects) object.tick(deltaTime);

	if (animationRate > 0.f && animationTarget != position) {
		position += (animationTarget - position) * std::atanf(animationRate * deltaTime) * (mat::pi / 2.f);
		if (mat::dist(animationTarget, position) < 0.1f) position = animationTarget;
	}
}
