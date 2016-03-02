//#include <math.h>

#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>

// Set up the publisher that publishes to the personPos topic
ros::Publisher personPos_pub;
geometry_msgs::Twist vel_msg;

// Subscriber callback function for recieving BodyArray msg from k2_client
void bodies_sub_cb(const k2_client::BodyArray msg){
  ROS_INFO_NAMED("personFollower", "personFollower: Received bodyArray");
  int s = -1;

  // If a person is being tracked by the Kinect, move towards this person.
  for(int i = 0; i <= 6; i++){
    if(msg.bodies[i].isTracked){
      ROS_INFO_NAMED("personFollower", "personFollower: Found tracked person");
      s = i;
      break;
    }
  }
  if(s != -1){	

    // jointPositions[3] is the persons "head". See startBody.cpp in k2_client package
    // int angle = atan(msg.bodies[s].jointPositions[3].position.x / msg.bodies[s].jointPositions[3].position.z);

    // Turn robot in order to make the robot face the person beiing tracked 
    if(msg.bodies[s].jointPositions[3].position.x > 0.5){
      vel_msg.angular.z = 0.3;
    } 
    else if(msg.bodies[s].jointPositions[3].position.x < -0.5){
      vel_msg.angular.z = -0.3;
    }
    // Move robot towards person as a function of the distance between robot and person. 
    if(msg.bodies[s].jointPositions[3].position.z > 1.2){
      vel_msg.linear.x = msg.bodies[s].jointPositions[3].position.z/10; //0.3;	
    }
  }
  // Publish velocity to RosAria cmd_vel
  personPos_pub.publish(vel_msg);
}

int main(int argc,char **argv){
  ros::init(argc,argv,"gestures");
  ros::NodeHandle n;

  // Set up the publisher that publishes to cmd_vel topic which RosAria subscribes to
  personPos_pub = n.advertise<geometry_msgs::Twist>("RosAria/cmd_vel",1);

  // Subsribe to topic "bodyArray" published by k2_klient package node startBody.cpp
  ros::Subscriber bodies_sub = n.subscribe("bodyArray", 1, bodies_sub_cb);

  ros::Rate loop_rate(20); //0.1

  // Run the ROS node
  ROS_INFO_NAMED("personFollower", "personFollower: Running ROS node...");
  while (ros::ok()){
    ros::spinOnce();
    loop_rate.sleep();
  }
  //ros::spin();

  ROS_INFO_NAMED("personFollower",  "personFollower: Quitting... \n" );

  return 0;
}   
