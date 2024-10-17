#include "textlabel.h"

#include <iostream>

int render(Textlabel *label) {
	freetype_render(*label, freetype_text_shader);

	return 0;
}

void lua_bsge_init_textlabel(sol::state &lua) {
	lua.new_usertype<Textlabel>("Textlabel",
								"color", &Textlabel::color,
								"position", &Textlabel::position,
								"scale", &Textlabel::scale,
								// "text", &Textlabel::text,
								"text", sol::property(&Textlabel::get_text, &Textlabel::set_text),
								"font", &Textlabel::font,
								"render", &render);
}
