#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_listener.h>

// Set up the publisher that publishes to the personPos topic
ros::Publisher personPos_pub;
// Set up a transform listner to recieve transformation data from rosarnl_node 
boost::shared_ptr<tf::TransformListener> listener;
geometry_msgs::PoseStamped personPos_msg;
geometry_msgs::PoseStamped personMapPos_msg;

// Subscriber callback function for recieving BodyArray msg from k2_client
void bodies_sub_cb(const k2_client::BodyArray msg){
  ROS_INFO_NAMED("personNavigator", "personNavigator: Received bodyArray");
  int s = -1;

  // Fill personPos_msg so that it can be published
  // jointPositions[3] is the persons "head". See startBody.cpp in k2_client package
  // personPos_msg.header.seq 

  // If a person is being tracked by the Kinect, move towards this person.
  for(int i = 0; i <= 6; i++){
    if(msg.bodies[i].isTracked){
      s = i;
      break;
    }
  }
  if(s != -1){
    // Set opp the PoseStamped msg to be sent to the rosarnl_node
    personPos_msg.header.stamp = msg.bodies[s].header.stamp;
    personPos_msg.header.frame_id = "base_link"; //msg.bodies[0].header.frame_id; //use "map" or "base" instead? does "base" exist?
    personPos_msg.pose.position.x = msg.bodies[s].jointPositions[3].position.x; // x is horizontal position
    personPos_msg.pose.position.z = msg.bodies[s].jointPositions[3].position.y; // y is vertical position
    personPos_msg.pose.position.y = msg.bodies[s].jointPositions[3].position.z; // z is distance from kinect (use insted of vertical?)
    personPos_msg.pose.orientation.x = 0;
    personPos_msg.pose.orientation.y = 0;
    personPos_msg.pose.orientation.z = 0; //msg.bodies[s].jointOrientations[3].orientation.z;
    personPos_msg.pose.orientation.w = 0; //msg.bodies[s].jointOrientations[3].orientation.w;

    personMapPos_msg.header.stamp = msg.bodies[s].header.stamp;
    personMapPos_msg.header.frame_id = "base_link"; //msg.bodies[0].header.frame_id; //use "map" or "base" instead? does "base" exist?
    personMapPos_msg.pose.position.x = msg.bodies[s].jointPositions[3].position.x; // x is horizontal position
    personMapPos_msg.pose.position.z = msg.bodies[s].jointPositions[3].position.y; // y is vertical position
    personMapPos_msg.pose.position.y = msg.bodies[s].jointPositions[3].position.z; // z is distance from kinect (use insted of vertical?)
    personMapPos_msg.pose.orientation.x = 0;
    personMapPos_msg.pose.orientation.y = 0;
    personMapPos_msg.pose.orientation.z = 0; //msg.bodies[s].jointOrientations[3].orientation.z;
    personMapPos_msg.pose.orientation.w = 0; //msg.bodies[s].jointOrientations[3].orientation.w;

    // Transform the personPos_msg to PersonMapPos_msg and publish. Kinect reference -> map reference
    try{
      //listner.lookupTransform("map", "base_link", ros::Time(0), transform);
      listener->transformPose("map", personPos_msg, personMapPos_msg);
    }
    catch (tf::TransformException ex){
      ROS_ERROR("%s",ex.what());
      ros::Duration(1.0).sleep();
    }

    // Publish person koordinate to rosarnl_node, move_base()
    personPos_pub.publish(personMapPos_msg);

    if(!personPos_pub){
      // Tell ROS of problem
    }
  }
}



int main(int argc,char **argv){
  ros::init(argc,argv,"personNavigator");
  ros::NodeHandle n;

  //tf::TransformListener listner;
  listener.reset(new tf::TransformListener);

  // --- From rosarnl_node: ---
  // /rosarnl_node/move_base_simple/goal
  // ros::Subscriber simple_goal_sub;

  // Set up topic move_base_simple/goal
  personPos_pub = n.advertise<geometry_msgs::PoseStamped>("rosarnl_node/move_base_simple/goal",1);

  // Subsribe to topic "bodyArray" published by k2_client package node startBody.cpp
  ros::Subscriber bodies_sub = n.subscribe("kinect2/bodyArray", 1, bodies_sub_cb);

  ros::Rate loop_rate(2); //0.1

  // Run ROS node
  ROS_INFO_NAMED("personNavigator", "personNavigator: Running ROS node...");
  while (ros::ok()){
    ros::spinOnce();
    loop_rate.sleep();
  }
  ROS_INFO_NAMED("personNavigator",  "personNavigator: Quitting... \n" );

  return 0;
}   



