bool vdbWasKeyPressed(vdbKey key)  { return keys::pressed[key]  && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbWasKeyReleased(vdbKey key) { return keys::released[key] && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbIsKeyDown(vdbKey key)      { return keys::down[key]     && !ImGui::GetIO().WantCaptureKeyboard; }
