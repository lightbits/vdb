### vdb - visual debugger

Lets you set breakpoints in your code and create interactive visualizations of your data and variables.

FAQ
---

#### How do I use this?

test.cpp contains several examples of the functionality, and a note on how to compile the file for Linux and Windows.

#### What's the GUI library?

The GUI is an [external library](https://github.com/ocornut/imgui/) by ocornut.

#### Can I change the font of the GUI?

Yes. Include a #define of VDB_CUSTOM_FONT before #include'ing vdb. For example

```
#define VDB_CUSTOM_FONT "C:/Windows/Fonts/SourceSansPro-SemiBold.ttf", 18.0f
```

#### There's a bug.

I know. Step through your program with a debugger (like gdb or vs) to find the error, and fix it.

#### I want to do X / I don't like that you do Y

Feel free to modify everything to your own needs.
