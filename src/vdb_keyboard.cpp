namespace keys
{
    static bool pressed[SDL_NUM_SCANCODES];
    static bool down[SDL_NUM_SCANCODES];
    static bool released[SDL_NUM_SCANCODES];
}

bool vdbWasKeyPressed(vdbKey key)  { return keys::pressed[key]  && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbWasKeyReleased(vdbKey key) { return keys::released[key] && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbIsKeyDown(vdbKey key)      { return keys::down[key]     && !ImGui::GetIO().WantCaptureKeyboard; }
