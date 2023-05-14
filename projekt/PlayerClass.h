#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gun.h"

class Player {
public:
	glm::vec3 position;
	glm::vec3 color;
	float jump_speed;
	float speed;
	bool jump_active = false;;

	float jumpTime = 0.0f;

	const float maxSpeed = 0.3f;
	const float max_jump_speed = 0.8f;


	Player()
	{
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		speed = maxSpeed;
		jump_speed = max_jump_speed;

	}

	void Move(glm::vec3 change)
	{
		if (glm::abs(position.x) < 20.0f && glm::abs(position.z) < 20.0f)
			position = position + speed * change;
		else
			position = glm::vec3(0.0f);
	}

	void Update(float t, float g, float yPos)
	{
		if (jump_active == true)
		{
			jumpTime += t;
			position.y += jump_speed * jumpTime + 0.4f * g * jumpTime * jumpTime;
			if (position.y < yPos)
			{
				jump_active = false;
				position.y = yPos;
				jumpTime = 0;
			}
		}
	}

	bool CheckBulletCollision(glm::vec3 bulletPos)
	{
		if (abs(position.x - bulletPos.x) < 0.5f && abs(position.y - bulletPos.y) < 0.5f && abs(position.z - bulletPos.z) < 0.5f)
			return true;
		return false;
	}

	int CheckEnemyCollision(glm::vec3 enemyPos)
	{
		if (abs(position.x - enemyPos.x) < 1 && abs(position.z - enemyPos.z) < 1
			&& position.y - enemyPos.y > 0.5f && position.y - enemyPos.y < 0.7f)
			return 2;
		else if (abs(position.x - enemyPos.x) < 1 && abs(position.y - enemyPos.y) < 0.5f && abs(position.z - enemyPos.z) < 1)
			return 1;
		return 0;
	}

};
