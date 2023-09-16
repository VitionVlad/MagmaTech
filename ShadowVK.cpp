#include <iostream>
#include "Engine.hpp"

int main(){
	Engine eng;
	eng.init("test");
	Mesh test;
	vertex vert[3];

	vert[0].position = glm::vec3(0.0, -0.5, -5);
	vert[1].position = glm::vec3(0.5, 0.5, -5);
	vert[2].position = glm::vec3(-0.5, 0.5, -5);

	vert[0].uv = glm::vec2(0, 0);
	vert[1].uv = glm::vec2(0, 0);
	vert[2].uv = glm::vec2(0, 0);

	vert[0].normal = glm::vec3(0, 0, 0);
	vert[1].normal = glm::vec3(0, 0, 0);
	vert[2].normal = glm::vec3(0, 0, 0);

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", vert, 3);
	while (eng.shouldterminate()) {
		eng.beginmainpass();
		test.Draw(eng);
		eng.endmainpass();
	}
	eng.terminate();
}