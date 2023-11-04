#include <iostream>

#include <vector>

#include <string>

#include <cmath>

#include "glm/glm.hpp"

class clickzone {
public:
	glm::vec2 pos = glm::vec2(0, 0);
	glm::vec2 size = glm::vec2(0, 0);
	int action(glm::vec2 pointerpos, bool clicked) {
		int state = 0;
		if (pointerpos.x >= pos.x && pointerpos.x <= pos.x + size.x && pointerpos.y >= pos.y && pointerpos.y <= pos.y + size.y) {
			state = 1;
			if (clicked == true) {
				state = 2;
			}
		}
		return state;
	}
};