# Updating ImGui

Follow the instructions below to update [Dear ImGui](https://github.com/ocornut/imgui) to the latest release:

* Copy the *.cpp files from the Dear ImGui repository into this directory, overwriting files.
* Update include/vdb/imgui.h
* Remove imconfig reference in imgui.h
* Remove anonymous namespace in imgui_freetype.cpp

In imgui_impl_opengl3.cpp:CheckShader, replace
```
    if ((GLboolean)status == GL_FALSE)
        ...
    if (log_length > 0)
        ...
'''

with

```
    if ((GLboolean)status == GL_FALSE)
    {
        ...
        if (log_length > 0)
            ...
    }
'''
