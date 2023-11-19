#include <iostream>

#include <vector>

#include <string>

#include <cmath>

#include "glm/glm.hpp"

class Bomb{
public:
	float radius = 1;
	float power = 1;
	glm::vec3 pos = glm::vec3(0, 0, 0);
	bool actioned = false;
};

class PhysEngine {
public:
	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec3 lpos = glm::vec3(0, 0, 0);
	glm::vec3 aabb = glm::vec3(0.3, 4, 0.3);
	glm::vec2 rot = glm::vec2(0, 0);
	float mass = 0.1;
	std::vector<Bomb> bombs{};
	int createNewBomb(glm::vec3 pos, float radius) {
		bombs.resize(bombs.size() + 1);
		bombs[bombs.size() - 1].pos.x = pos.x;
		bombs[bombs.size() - 1].pos.x = pos.y;
		bombs[bombs.size() - 1].pos.z = pos.z;
		bombs[bombs.size() - 1].radius = radius;
		return bombs.size() - 1;
	}
};

class MeshPhys {
private:
	glm::vec4 center;
	bool inRange(int low, int high, int mx) {
		return ((mx - high) * (mx - low) <= 0);
	}
	float dist = 0;
	glm::vec3 laabb = glm::vec3(0, 0, 0);
public:
	bool canbedestroyed = true;
	bool scalebool = false;
	bool rotbool = false;
	bool transbool = false;
	bool collision = true;
	int isinteracting = 0;
	float resistance = 0.5;
	bool flipy = true;
	bool flipx = false;
	bool flipz = false;
	void physWork(PhysEngine& eng, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3, glm::mat4& mtrans, glm::mat4& mrx, glm::mat4& mry, glm::mat4& mrz, glm::mat4& ms) {
		laabb = glm::vec3(0, 0, 0);
		isinteracting = 0;
		center.x = (v1.x + v2.x + v3.x) / 3;
		center.y = (v1.y + v2.y + v3.y) / 3;
		center.z = (v1.z + v2.z + v3.z) / 3;
		if (flipy) {
			center.y = -center.y;
		}
		if (flipx) {
			center.x = -center.x;
		}
		if (flipz) {
			center.z = -center.z;
		}
		center.w = 1.0f;
		if (scalebool) {
			center = ms * center;
		}
		if (rotbool) {
			center = mrx * mry * mrz * center;
		}
		if (transbool) {
			center = mtrans * center;
		}
		if (canbedestroyed) {
			for (int i = 0; i != eng.bombs.size(); i++) {
				dist = sqrt(pow(eng.bombs[i].pos.x - center.x, 2) + pow(eng.bombs[i].pos.y - center.y, 2) + pow(eng.bombs[i].pos.z - center.z, 2));
				if (dist <= eng.bombs[i].radius && eng.bombs[i].actioned == false && eng.bombs[i].power >= resistance) {
					v1 = glm::vec3(0, 0, 0);
					v2 = glm::vec3(0, 0, 0);
					v3 = glm::vec3(0, 0, 0);
				}
			}
		}
		if (inRange(-eng.pos.x - eng.aabb.x, -eng.pos.x + eng.aabb.x, center.x) &&
			inRange(-eng.pos.z - eng.aabb.z, -eng.pos.z + eng.aabb.z, center.z) &&
			center.y >= eng.pos.y - eng.aabb.y - 0.1 &&
			center.y <= eng.pos.y) {
			if (collision) {
				eng.pos.y = eng.lpos.y;
			}
			isinteracting = 1;
			if (center.y > eng.pos.y - eng.aabb.y / 2) {
				if (collision) {
					eng.pos.x = eng.lpos.x;
					eng.pos.z = eng.lpos.z;
				}
				isinteracting = 2;
			}
		}
	}
};