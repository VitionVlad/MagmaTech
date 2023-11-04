#include <iostream>

#include <string>

#include <cmath>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "glm/glm.hpp"

class AudioSource {
private:
	ma_result result{};
	ma_sound sound{};
public:
	glm::vec3 pos = glm::vec3(0, 0, 0);
	bool inspace = true;
	float maxdist = 1;
	void create(ma_engine& eng, std::string path) {
		result = ma_sound_init_from_file(&eng, path.c_str(), 0, NULL, NULL, &sound);
		if (result != MA_SUCCESS) {
			std::cout << "error: failed to init miniaudio sound, error code " << result;
			exit(result);
		}
	}
	void play(float volume, glm::vec3 playerpos) {
		float dist = sqrt(pow(playerpos.x - pos.x, 2) + pow(playerpos.y - pos.y, 2) + pow(playerpos.z - pos.z, 2));
		if (dist > maxdist) {
			ma_sound_set_volume(&sound, volume * (maxdist / dist));
		}
		else {
			ma_sound_set_volume(&sound, 0.0001);
		}
		if (!ma_sound_is_playing(&sound)) {
			ma_sound_start(&sound);
		}
	}
	void stop() {
		ma_sound_stop(&sound);
	}
};