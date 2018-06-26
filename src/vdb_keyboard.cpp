bool vdbKeyPressed(vdbKey key)  { return vdb.key_pressed[key]  && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbKeyDown(vdbKey key)     { return vdb.key_down[key]     && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbKeyReleased(vdbKey key) { return vdb.key_released[key] && !ImGui::GetIO().WantCaptureKeyboard; }
