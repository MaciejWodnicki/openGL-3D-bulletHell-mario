#pragma once

#include "bullet.h"
#include <memory>
#include <iostream>

class Gun {
public:
	glm::vec3 location;
	glm::vec3 direction;
	std::unique_ptr<Bullet> bullet;
	float bulletSpeed = 0.05f;
	
	float cooldown = 2.5f;
	Gun() {};

	Gun(glm::vec3 location, glm::vec3 direction)
	{
		this->location = location;	
		this->direction = direction;
		bullet = nullptr;
	}

	std::unique_ptr<Bullet> shoot()
	{
		bullet = std::make_unique<Bullet>(location, direction, bulletSpeed);
		bullet->shootAt(direction);
		return std::move(bullet);
	}

	bool isShot()
	{
		if (bullet == nullptr)
			return false;
		return bullet->getState();
	}

	glm::vec3 getBulletPos()
	{
		if (bullet == nullptr)
			return glm::vec3(0.0f);
		return bullet->getPos();
	}

};
