#pragma once
#include "ECS\System.h"
#include "ECS/Components.h"
#include "ECS/ECSCoordinator.h"
class LightSystem : public System {
private:

public:
	virtual void update(float dt) override;

};