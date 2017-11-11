## ROS example

This example shows how you could use VDB in a ROS project, where one node (app.cpp) does some work, while another node (gui.cpp) visualizes its results and also provides a GUI for adjusting parameters live.

To try out the example, do the following:

```bash
# Install SDL2 (if you haven't already)
sudo apt-get install libsdl2-dev

# Clone vdb into your catkin workspace's src directory
cd ~catkin_ws/src
git clone https://github.com/lightbits/vdb.git vdb

# Move files in example_ros to a new node
mv vdb/example_ros ./example_vdb

# Move vdb source files into that node
mv vdb/src ./example_vdb/vdb

# Compile nodes
cd ..
catkin_make

# Run example
roscore &
rosrun example_vdb app &
rosrun example_vdb gui
```
