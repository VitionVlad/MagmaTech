#include <iostream>
#include "Engine.hpp"

Engine eng;

const float speed = 0.1f;

bool mouseattached = true;

void movecallback() {
    int state = glfwGetKey(eng.ren.window, GLFW_KEY_W);
    if (state == GLFW_PRESS) { //w
        eng.ren.pos.z += cos(eng.ren.rot.x) * cos(eng.ren.rot.y) * speed;
        eng.ren.pos.x += cos(eng.ren.rot.x) * sin(eng.ren.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_A);
    if (state == GLFW_PRESS) { // a
        eng.ren.pos.x += cos(eng.ren.rot.x) * cos(eng.ren.rot.y) * speed;
        eng.ren.pos.z -= cos(eng.ren.rot.x) * sin(eng.ren.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_S);
    if (state == GLFW_PRESS) { // s
        eng.ren.pos.z -= cos(eng.ren.rot.x) * cos(eng.ren.rot.y) * speed;
        eng.ren.pos.x -= cos(eng.ren.rot.x) * sin(eng.ren.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_D);
    if (state == GLFW_PRESS) { //d
        eng.ren.pos.x -= cos(eng.ren.rot.x) * cos(eng.ren.rot.y) * speed;
        eng.ren.pos.z += cos(eng.ren.rot.x) * sin(eng.ren.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_ESCAPE);
    if (state == GLFW_PRESS) { //d
        if (mouseattached) {
            glfwSetInputMode(eng.ren.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouseattached = false;
        }
        else {
            glfwSetInputMode(eng.ren.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseattached = true;
        }
    }
}

int main(){
    glm::dvec2 mousepos;
    eng.ren.resolutionscale = 1;
	eng.init("test");
    glfwSetInputMode(eng.ren.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    eng.ren.ShadowOrtho = true;
    eng.ren.sFov = 5;
    eng.ren.useShadowLookAt = true;
    eng.ren.ShadowLookAt = glm::vec3(0, 0, 0);
    Object test;

    std::string texpaths[2] = { "data/t.ppm" , "data/t.ppm" };
    test.mesh.scale.y = -1;
    test.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", "data/m.obj", texpaths, 2);
	while (eng.ren.shouldterminate()) {
        glfwGetCursorPos(eng.ren.window, &mousepos.x, &mousepos.y);
        eng.ren.rot.y = mousepos.x / eng.ren.resolution.x;
        eng.ren.rot.x = -mousepos.y / eng.ren.resolution.y;
        movecallback();
        eng.beginShadowPass();

        test.Draw(eng);

        eng.beginMainPass();

		test.Draw(eng);

		eng.endRender();
	}
	eng.terminate();
}