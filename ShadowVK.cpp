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
    eng.ren.useShadowLookAt = true;
    eng.ren.ShadowLookAt = glm::vec3(0, 0, 0);
    eng.ren.ShadowPos = glm::vec3(-50, -50, 50);
    eng.ren.sFov = 30;
    eng.ren.pos.z = -2;
    eng.ren.pos.y = 4;
    eng.ren.ubo.lightColor[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0);
    eng.ren.ubo.lightPos[0] = glm::vec4(1, -1, 1, 1);
    Object test;
    Object cube;

    std::string cubemap[6] = { "data/right.ppm" , "data/left.ppm", "data/top.ppm", "data/bottom.ppm" , "data/front.ppm", "data/back.ppm" };
    std::string texpaths[3] = { "data/t.ppm" , "data/spec.ppm", "data/normal.ppm"};
    test.mesh.scale.y = -1;
    test.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;
    test.mesh.shadowcullmode = VK_CULL_MODE_BACK_BIT;

	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv", "data/m.obj", texpaths, 3, cubemap, 1);
    cube.mesh.scale = glm::vec3(1000, 1000, 1000);
    cube.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;
    cube.create(eng, "data/raw/vertc.spv", "data/raw/fragc.spv", "data/cube.obj", texpaths, 3, cubemap, 1);
	while (eng.ren.shouldterminate()) {
        glfwGetCursorPos(eng.ren.window, &mousepos.x, &mousepos.y);
        eng.ren.rot.y = mousepos.x / eng.ren.resolution.x;
        eng.ren.rot.x = -mousepos.y / eng.ren.resolution.y;
        movecallback();
        eng.ren.sFov = eng.ren.fov;
        eng.beginShadowPass();

        test.Draw(eng);

        eng.beginMainPass();

		test.Draw(eng);
        cube.Draw(eng);

		eng.endRender();
	}
	eng.terminate();
}