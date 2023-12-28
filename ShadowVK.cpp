#include <iostream>
#include "Engine.hpp"

Engine eng;

const float speed = 0.1f;

bool mouseattached = true;

void movecallback() {
    int state = glfwGetKey(eng.ren.window, GLFW_KEY_W);
    if (state == GLFW_PRESS) { //w
        eng.pos.z += cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.x += cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_A);
    if (state == GLFW_PRESS) { // a
        eng.pos.x += cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.z -= cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_S);
    if (state == GLFW_PRESS) { // s
        eng.pos.z -= cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.x -= cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_D);
    if (state == GLFW_PRESS) { //d
        eng.pos.x -= cos(eng.rot.x) * cos(eng.rot.y) * speed;
        eng.pos.z += cos(eng.rot.x) * sin(eng.rot.y) * -speed;
    }
    state = glfwGetKey(eng.ren.window, GLFW_KEY_E);
    if (state == GLFW_PRESS) { //d
        eng.peng.bombs[0].pos = glm::vec3(-eng.pos.x, eng.pos.y, eng.pos.z);
        eng.peng.bombs[0].actioned = false;
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
    eng.pos.z = -2;
    eng.pos.y = 8;
	eng.init("test");
    glfwSetInputMode(eng.ren.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    eng.ren.ShadowOrtho = true;
    eng.ren.useShadowLookAt = true;
    eng.ren.ShadowLookAt = glm::vec3(0, 0, 0);
    eng.ren.ShadowPos = glm::vec3(-50, -50, 50);
    eng.ren.sFov = 30;
    eng.ren.ubo.lightColor[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0);
    eng.ren.ubo.lightPos[0] = glm::vec4(1, -1, 1, 1);
    eng.peng.createNewBomb(glm::vec3(eng.pos.x, eng.pos.y, eng.pos.z), 5);
    eng.peng.bombs[0].actioned = true;
    Object test;
    Object cube;

    std::string cubemap[6] = { "data/right.ppm" , "data/left.ppm", "data/top.ppm", "data/bottom.ppm" , "data/front.ppm", "data/back.ppm" };
    std::string texpaths[3] = { "data/t.ppm" , "data/spec.ppm", "data/normal.ppm"};
    test.scale.y = -1;
    test.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;
    test.mesh.shadowcullmode = VK_CULL_MODE_BACK_BIT;

	test.createNoCube(eng, "data/raw/vert.spv", "data/raw/frag.spv", "data/m.obj", texpaths, 3/*, "data/test.mp3", 1*/);
    cube.scale = glm::vec3(1000, 1000, 1000);
    cube.mesh.cullmode = VK_CULL_MODE_FRONT_BIT;
    cube.createNoTex(eng, "data/raw/vertc.spv", "data/raw/fragc.spv", "data/cube.obj", cubemap, 1);
    cube.enablecollisiondetect = false;

    uiText text1;
    text1.create(eng, 100, 52, "data/symbols/sym.ppm", "data/raw/uivert.spv", "data/raw/uifrag.spv", "ABCDEFGHIKLMNOPQRTUVWXYZabcdefghiklmnopqrstuvwxyz0123456789,.;: ");
    text1.rsymsize = 25;

    int state;

    std::cout << "render begin" << std::endl;
 	while (eng.ren.shouldterminate()) {
        glfwGetCursorPos(eng.ren.window, &mousepos.x, &mousepos.y);
        eng.rot.y = mousepos.x / eng.ren.resolution.x;
        eng.rot.x = -mousepos.y / eng.ren.resolution.y;
        movecallback();
        state = glfwGetMouseButton(eng.ren.window, GLFW_MOUSE_BUTTON_LEFT);
        eng.beginShadowPass();

        test.Draw(eng);

        eng.beginMainPass();

		test.Draw(eng);
        cube.Draw(eng);
        text1.Draw(eng, glm::vec2(0, 0), "Hello");

		eng.endRender();
	}
	eng.terminate();
}