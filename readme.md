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

Gallery
-------

Here are some visualizations that I've made while working with computer vision stuff. An important thing to note here, is that the visualizations themselves only took about 1-5 minutes to write. And that's my goal with this library --- to make visualizing complex data as easy and quick as possible, while still giving you power to do anything.

#### Adjusting parameters of an edge filter
![](img/img1.png)

#### Computing the possible circles intersecting two points
![](img/img2.png)

#### Visualizing image gradients and a circle detector
![](img/img3.png)

#### Verifying my math for projecting image coordinates to the world
![](img/img4.png)

#### A quick tool to select points for fisheye calibration
I primarily made this because the interface for MATLAB's calibration tool was so awful to use, so I figured I might as well do it myself.

![](img/img5.png)

#### Verifying the fisheye calibration
Here I could adjust the fisheye parameters and the camera rotation/position, and see whether the projected grid lines up with the checkerboard.

![](img/img6.png)

#### Determining the dominant directions of gradients
Here I compute a Gauss-smoothed histogram over the angles of each image gradient, to determine which angles are more dominant. That in turn lets me extract the two directions of the grid lines. Visualizing this was helpful, because it turns out that this fails really quickly, once you get bad input.

![](img/img7.png)

#### Making a user interface for a path controller

![](img/img8.png)

#### Mean-shift cluster detection

Here I'm drawing the output of the mean-shift algorithm after it converged to the regions with high density. I also included a note to myself, pointing out that the algorithm had a hard time converging to a region of high density within a thin strip.

The neat thing with this was that I visualized the output step-by-step, and that actually let me discover a bug in my code. The way I found it was that I noticed that some of the windows were oscillating every other iteration. I though that was strange, so I printed out the number of points in each window, and, to my own dismay, I realized that I forgot to clear the counter per iteration.

But, despite the fact that there was a bug, the algorithm still gave reasonable results. If I hadn't visualized each step, that bug would have probably given me headaches further down the line.

![](img/img9.png)

#### Hough transform line detection

![](img/img14.png)

Here I'm drawing all estimated lines, in turn, from a Hough transform, and their supporting points. One line in particular seemed strange, in that all its supporting points had really different image gradients, even though I had a check to reject such points.

![](img/img13.png)

Turns out the bug was a sign error in the dot product.
