#include <ros/ros.h>
#include <std_msgs/Float32.h>

float param_x = 1.0f;
float param_y = 1.0f;

void callback_x(std_msgs::Float32 msg) {
    param_x = msg.data;
}

void callback_y(std_msgs::Float32 msg) {
    param_y = msg.data;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "app");
    ros::NodeHandle node;
    ros::Subscriber sub_x = node.subscribe("/example_vdb/x", 1, callback_x);
    ros::Subscriber sub_y = node.subscribe("/example_vdb/y", 1, callback_y);
    ros::Publisher pub_result = node.advertise<std_msgs::Float32>("/example_vdb/result", 1);

    while (ros::ok())
    {
        // Get latest parameters from callbacks
        ros::spinOnce();

        // Example of some algorithm using parameters to do stuff
        float result;
        {
            result = param_x + param_y;
            ros::Rate rate(5.0); // pretend that it took a long time
            rate.sleep();
        }
        {
            std_msgs::Float32 msg;
            msg.data = result;
            pub_result.publish(msg);
        }
    }

    return 0;
}
