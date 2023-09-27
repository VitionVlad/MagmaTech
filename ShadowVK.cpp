#include <iostream>
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
    eng.ShadowOrtho = true;
    eng.sFov = 5;
    eng.useShadowLookAt = true;
    eng.ShadowLookAt = glm::vec3(0, 0, 0);
    eng.ShadowPos = glm::vec3(-10, -10, -10);
	Mesh test;

    std::string texpaths[2] = { "data/t.ppm" , "data/t.ppm" };

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", "data/m.obj", texpaths, 2);
	while (eng.shouldterminate()) {
        glfwSetInputMode(eng.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(eng.window, &mousepos.x, &mousepos.y);
        eng.rot.y = mousepos.x / eng.resolution.x;
        eng.rot.x = -mousepos.y / eng.resolution.y;
        movecallback();
		eng.beginRender();
        eng.beginShadowPass();

        test.Draw(eng);

        eng.beginMainPass();

		test.Draw(eng);

		eng.endRender();
	}
	eng.terminate();
}