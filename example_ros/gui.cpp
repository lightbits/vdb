#include "vdb/vdb.h" // See readme.md for how to bundle vdb with your project
#include <ros/ros.h>
#include <std_msgs/Float32.h>

float last_result = 0.0f;

void callback_result(std_msgs::Float32 msg)
{
    last_result = msg.data;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "gui");
    ros::NodeHandle node;
    ros::Subscriber sub_result = node.subscribe("/example_vdb/result", 1, callback_result);
    ros::Publisher pub_x = node.advertise<std_msgs::Float32>("/example_vdb/x", 1);
    ros::Publisher pub_y = node.advertise<std_msgs::Float32>("/example_vdb/y", 1);

    float param_x = 1.0f;
    float param_y = 1.0f;

    VDBB("GUI");
    {
        // Get latest result from callbacks
        ros::spinOnce();

        // Display result (imagine this being something more advanced...
        // e.g. a camera image, a computer vision algorithm output, a
        // LIDAR range array, etc.)
        Text("Last result: %.2f", last_result);

        // Tweak parameters (imagine there being more of these...)
        DragFloat("x", &param_x);
        DragFloat("y", &param_y);

        // Publish parameters
        { std_msgs::Float32 msg; msg.data = param_x; pub_x.publish(msg); }
        { std_msgs::Float32 msg; msg.data = param_y; pub_y.publish(msg); }
    }
    VDBE();
    return 0;
}
