#include "textlabel.h"

#include <iostream>

int render(Textlabel label) {

	freetype_render(label, freetype_text_shader);

	return 0;
}

void lua_bsge_init_textlabel(sol::state &lua) {
	lua.new_usertype<Textlabel>("Textlabel",
								"color", &Textlabel::color,
								"position", &Textlabel::position,
								"scale", &Textlabel::scale,
								"text", &Textlabel::text,
								"font", &Textlabel::font,
								"render", render);

	lua.new_usertype<glm::vec3>("Vec3",
								"x", &glm::vec3::x,
								"y", &glm::vec3::y,
								"z", &glm::vec3::z);
}
