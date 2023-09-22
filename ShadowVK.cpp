#include <iostream>
#include "ResourceLoader.hpp"
#include "Engine.hpp"

Engine eng;

const float speed = 0.1f;

void movecallback() {
    int state = glfwGetKey(eng.window, GLFW_KEY_W);
    if (state == GLFW_PRESS) { //w
        eng.pos.z += cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.x += cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.window, GLFW_KEY_A);
    if (state == GLFW_PRESS) { // a
        eng.pos.x += cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.z -= cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.window, GLFW_KEY_S);
    if (state == GLFW_PRESS) { // s
        eng.pos.z -= cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.x -= cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.window, GLFW_KEY_D);
    if (state == GLFW_PRESS) { //d
        eng.pos.x -= cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.z += cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
}

int main(){
    glm::dvec2 mousepos;
	eng.init("test");
	Mesh test;

    Loader objtest;
    objtest.loadobj("data/m.obj");
    objtest.loadppm("data/t.ppm");

    std::cout << objtest.textureResolution.x << " " << objtest.textureResolution.y << std::endl;

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", objtest.vertex.data(), objtest.uv.data(), objtest.normals.data(), objtest.vertex.size(), objtest.pixels.data(), objtest.textureResolution, 1);
	while (eng.shouldterminate()) {
        glfwSetInputMode(eng.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(eng.window, &mousepos.x, &mousepos.y);
        eng.rot.y = mousepos.x / eng.resolution.x;
        eng.rot.x = -mousepos.y / eng.resolution.y;
        movecallback();
		eng.beginmainpass();
		test.Draw(eng);
		eng.endmainpass();
	}
	eng.terminate();
}