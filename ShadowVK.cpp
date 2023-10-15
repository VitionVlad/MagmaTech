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
    eng.ren.ShadowOrtho = false;
    eng.ren.useShadowLookAt = false;
    eng.ren.ShadowLookAt = glm::vec3(0, 0, 0);
    eng.ren.pos.z = -2;
    eng.ren.pos.y = 4;
    Object test;

    std::string texpaths[6] = { "data/t.ppm" , "data/spec.ppm", "data/normal.ppm" , "data/t.ppm", "data/t.ppm" , "data/t.ppm" };
    test.mesh.scale.y = -1;
    test.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;
    test.mesh.shadowcullmode = VK_CULL_MODE_BACK_BIT;

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", "data/m.obj", texpaths, 3, texpaths, 1);
	while (eng.ren.shouldterminate()) {
        glfwGetCursorPos(eng.ren.window, &mousepos.x, &mousepos.y);
        eng.ren.rot.y = mousepos.x / eng.ren.resolution.x;
        eng.ren.rot.x = -mousepos.y / eng.ren.resolution.y;
        movecallback();
        eng.ren.sFov = eng.ren.fov;
        eng.ren.ShadowPos = eng.ren.pos;
        eng.ren.ShadowRot = eng.ren.rot;
        eng.ren.ubo.lightColor[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0);
        eng.ren.ubo.lightPos[0] = glm::vec4(-eng.ren.pos.x, 4, -eng.ren.pos.z, 0);
        eng.beginShadowPass();

        test.Draw(eng);

        eng.beginMainPass();

		test.Draw(eng);

		eng.endRender();
	}
	eng.terminate();
}