// Subset of scancodes copied from SDL_scancodes.h
vdbKey VDB_KEY_INVALID = 0;
vdbKey VDB_KEY_A = 4;
vdbKey VDB_KEY_B = 5;
vdbKey VDB_KEY_C = 6;
vdbKey VDB_KEY_D = 7;
vdbKey VDB_KEY_E = 8;
vdbKey VDB_KEY_F = 9;
vdbKey VDB_KEY_G = 10;
vdbKey VDB_KEY_H = 11;
vdbKey VDB_KEY_I = 12;
vdbKey VDB_KEY_J = 13;
vdbKey VDB_KEY_K = 14;
vdbKey VDB_KEY_L = 15;
vdbKey VDB_KEY_M = 16;
vdbKey VDB_KEY_N = 17;
vdbKey VDB_KEY_O = 18;
vdbKey VDB_KEY_P = 19;
vdbKey VDB_KEY_Q = 20;
vdbKey VDB_KEY_R = 21;
vdbKey VDB_KEY_S = 22;
vdbKey VDB_KEY_T = 23;
vdbKey VDB_KEY_U = 24;
vdbKey VDB_KEY_V = 25;
vdbKey VDB_KEY_W = 26;
vdbKey VDB_KEY_X = 27;
vdbKey VDB_KEY_Y = 28;
vdbKey VDB_KEY_Z = 29;
vdbKey VDB_KEY_1 = 30;
vdbKey VDB_KEY_2 = 31;
vdbKey VDB_KEY_3 = 32;
vdbKey VDB_KEY_4 = 33;
vdbKey VDB_KEY_5 = 34;
vdbKey VDB_KEY_6 = 35;
vdbKey VDB_KEY_7 = 36;
vdbKey VDB_KEY_8 = 37;
vdbKey VDB_KEY_9 = 38;
vdbKey VDB_KEY_0 = 39;
vdbKey VDB_KEY_RETURN = 40;
vdbKey VDB_KEY_ESCAPE = 41;
vdbKey VDB_KEY_BACKSPACE = 42;
vdbKey VDB_KEY_TAB = 43;
vdbKey VDB_KEY_SPACE = 44;
vdbKey VDB_KEY_F1 = 58;
vdbKey VDB_KEY_F2 = 59;
vdbKey VDB_KEY_F3 = 60;
vdbKey VDB_KEY_F4 = 61;
vdbKey VDB_KEY_F5 = 62;
vdbKey VDB_KEY_F6 = 63;
vdbKey VDB_KEY_F7 = 64;
vdbKey VDB_KEY_F8 = 65;
vdbKey VDB_KEY_F9 = 66;
vdbKey VDB_KEY_F10 = 67;
vdbKey VDB_KEY_F11 = 68;
vdbKey VDB_KEY_F12 = 69;
vdbKey VDB_KEY_HOME = 74;
vdbKey VDB_KEY_PAGEUP = 75;
vdbKey VDB_KEY_DELETE = 76;
vdbKey VDB_KEY_END = 77;
vdbKey VDB_KEY_PAGEDOWN = 78;
vdbKey VDB_KEY_RIGHT = 79;
vdbKey VDB_KEY_LEFT = 80;
vdbKey VDB_KEY_DOWN = 81;
vdbKey VDB_KEY_UP = 82;
vdbKey VDB_KEY_LCTRL = 224;
vdbKey VDB_KEY_LSHIFT = 225;
vdbKey VDB_KEY_LALT = 226;
vdbKey VDB_KEY_LGUI = 227;
vdbKey VDB_KEY_RCTRL = 228;
vdbKey VDB_KEY_RSHIFT = 229;
vdbKey VDB_KEY_RALT = 230;
vdbKey VDB_KEY_RGUI = 231;

namespace keys
{
    static bool pressed[VDB_NUM_KEYS];
    static bool down[VDB_NUM_KEYS];
    static bool released[VDB_NUM_KEYS];
}

bool vdbWasKeyPressed(vdbKey key)  { if (key >= 0 && key < VDB_NUM_KEYS) return keys::pressed[key]  && !ImGui::GetIO().WantCaptureKeyboard; return false; }
bool vdbWasKeyReleased(vdbKey key) { if (key >= 0 && key < VDB_NUM_KEYS) return keys::released[key] && !ImGui::GetIO().WantCaptureKeyboard; return false; }
bool vdbIsKeyDown(vdbKey key)      { if (key >= 0 && key < VDB_NUM_KEYS) return keys::down[key]     && !ImGui::GetIO().WantCaptureKeyboard; return false; }
