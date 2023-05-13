#pragma once
#include <glm/glm.hpp>


class Bullet {

private:
	glm::vec3 position;
	glm::vec3 direction;
	float velocity;
	bool isShot;

public:

	Bullet(glm::vec3 position, glm::vec3 direction, float velocity) : direction(direction), position(position), velocity(velocity) 
	{
		isShot = false;
	}

	void shootAt(glm::vec3 direction)
	{
		isShot = true;
		this->direction = direction;
	}

	void updateBulletPos()
	{
		position += direction * velocity;
	}

	glm::vec3 getPos()
	{
		return position;
	}

	bool getState()
	{
		return isShot;
	}

};