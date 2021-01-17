#include "ReSDL/ReSDLTypes.h"

namespace ReSDL {

const Color Color::Black = {0, 0, 0, 255};
const Color Color::Grey = {128, 128, 128, 255};
const Color Color::DarkGrey = (Color::Grey * 0.5).withAlpha(255);
const Color Color::LightGrey = (Color::Grey * 1.5).withAlpha(255);
const Color Color::White = {255, 255, 255, 255};

const Color Color::Red = {255, 0, 0, 255};
const Color Color::Green = {0, 255, 0, 255};
const Color Color::Blue = {255, 0, 0, 255};
const Color Color::Yellow = {255, 255, 0, 255};
const Color Color::Cyan = {0, 255, 255, 255};
const Color Color::Magenta = {255, 0, 255, 255};
const Color Color::DarkBlue = (Color::Blue * 0.5).withAlpha(255);
const Color Color::LightBlue = max(Color::Blue, Color::LightGrey);

}