#pragma once
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp >
#include <iostream>

class Enemy {
public:
	glm::vec3 position;
	glm::vec3 color;

	float baseSpeed = 0.1f;
	float speed;
	bool isMoving = false;
	glm::vec3 nextPosition;

	Gun gun;
	float lastShot = 0.0f;

	bool isDead = false;


	Enemy(glm::vec3 spawnPosition)
	{
		position = spawnPosition;
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		speed = baseSpeed;
	}

	std::unique_ptr<Bullet> RandomRoam(glm::vec3 target, float time, float minXY = -10, float maxXY = 10)
	{
		if (!isMoving)
		{
			auto randVec2 = glm::linearRand(glm::vec2(minXY), glm::vec2(maxXY));
			speed = glm::gaussRand(0.02f, 0.06f);
			nextPosition = glm::vec3(randVec2.x, position.y, randVec2.y);
			isMoving = true;
		}
		if (glm::distance(position, nextPosition) < 2.0f)
			isMoving = false;
		
		position = position + speed * glm::normalize((glm::vec3(nextPosition) - position));
		return ShootAt(target, time);
	}

	std::unique_ptr<Bullet> ShootAt(glm::vec3 target, float time)
	{
		if (time - lastShot < gun.cooldown+glm::linearRand(-2,2))
			return std::unique_ptr<Bullet>();

		lastShot = time;

		glm::vec3 shootDirection = glm::normalize(target - position);

		float angle = glm::acos(glm::length(glm::vec2(shootDirection.x, shootDirection.z)));

		if (angle > 0.3f)
			return std::unique_ptr<Bullet>();

		gun = Gun(glm::vec3(position.x,position.y+0.2f, position.z), glm::normalize(target - position));
		return gun.shoot();
	}
};
