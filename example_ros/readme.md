## ROS example

This example shows how you could use VDB in a ROS project, where one node (app) is doing some processing, while another node (gui) is visualizing its results, and also providing a GUI for adjusting parameters live.

You can try out the example by copying the contents of this directory into a new node in your catkin workspace, and copying the VDB [src](../src) directory with it.

After compiling with ```catkin_make``` you can run the example by typing in your terminal:

```
roscore &
rosrun example_vdb app &
rosrun example_vdb gui
```
