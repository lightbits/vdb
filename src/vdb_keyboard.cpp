bool vdbIsKeyPressed(vdbKey key)  { return vdb.key_pressed[key]  && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbIsKeyDown(vdbKey key)     { return vdb.key_down[key]     && !ImGui::GetIO().WantCaptureKeyboard; }
bool vdbIsKeyReleased(vdbKey key) { return vdb.key_released[key] && !ImGui::GetIO().WantCaptureKeyboard; }
