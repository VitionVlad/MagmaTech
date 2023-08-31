#include <iostream>
#include "Engine.hpp"


int main(){
	Engine eng;
	eng.uselayer = false;
	eng.init("test");
	Mesh test;
	test.create(eng, "data/raw/vert.spv", "data/raw/frag.spv");
	while (eng.shouldterminate()) {
		eng.beginmainpass();
		test.Draw(eng);
		eng.endmainpass();
	}
	eng.terminate();
}