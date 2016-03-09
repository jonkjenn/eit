#include "k2_client/BodyArray.h"
#include "ros/ros.h"
#include <vector>
#include <string>
#include "rosaria/BumperState.h"
#include "std_msgs/String.h"



void bodies_sub_cb(const k2_client::BodyArray msg)
{
  //gjenkjenne gesturer fra kinnect: 
  

  //happy or neutral
  for (const auto& body : msg.bodies){
      
    if (body.expressions.happy){
      ROS_INFO("The user eats happymeal ");
    }
      
    else if (body.expressions.neutral){
      ROS_INFO("The user is neutral ");
    }
    
    if(body.activities.eyeleftClosed and body.activities.eyeRightClosed){
      ROS_INFO("Eye closed af");
    }
  
    if(body.appearance.wearingGlasses){
      ROS_INFO("Wearing Glases");
    }
  }
    
}

int main(int argc, char **argv)
{
  /  ros::init(argc, argv, "listener");
  ros::NodeHandle n;

  ros::Subscriber sub = n.subscribe("head/kinect2/bodyArray", 1000, bodies_sub_cb);


    ros::spin();

  return 0;
}

