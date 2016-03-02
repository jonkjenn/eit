#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_listener.h>
#include <signal.h>
#include <string>
//#include "pid.h"

using namespace std;
//#include <algorithm> // min and max

ros::Publisher personPos_pub;
geometry_msgs::Twist vel_msg;

ros::Time prev_time;
float prev_vel_z;
float vel_inc = 0.05;
float vel_target = 0.3;
//pid pid(0.30,0.5,0.1,0.1,0,0.3);


int s = -1;
bool aboveAngle = false;
bool aboveLength = false;
bool prevAboveAngle = false;
bool prevAboveLength = false;

//int count = 0;
int underLenCount = 0;

void watchdog(int sig){
  vel_msg.linear.x = 0; 
  vel_msg.linear.y = 0; 
  vel_msg.linear.z = 0; 
  vel_msg.angular.x = 0;
  vel_msg.angular.y = 0;
  vel_msg.angular.z = 0;
  personPos_pub.publish(vel_msg);
}

void pose_sub_cb(const nav_msgs::Odometry::ConstPtr& msg){
  ROS_INFO_STREAM("Got odometry");

  float vel_x = msg->twist.twist.linear.x;
  float der_vel =  vel_x - prev_vel_x;

  ros::Time t = msg->header.stamp.to_sec();

  float sec = (t-prev_time).to_sec();

  if(vel_x - vel_target > -vel_inc){
    vel_x+=vel_inc;
  }
  else if(vel_x - vel_target < vel_inc){
    vel_x-=vel_inc;
  }

  prev_vel_x = msg->twist.twist.linear.x;
  prev_time = t;
}

void bodies_sub_cb(const k2_client::BodyArray msg){
  //ROS_INFO_NAMED("personFollower", "personFollower: Received bodyArray");

  alarm(1);
  //++count;

  vel_msg.linear.y = 0; 
  vel_msg.linear.z = 0; 
  vel_msg.angular.x = 0;
  vel_msg.angular.y = 0;

  // Continue to track the same person, or find a new person to track (if any)
  if(s != -1){
    if(msg.bodies[s].isTracked == 0){
      s = -1;
      ROS_INFO_NAMED("personFollower", "personFollower: tracking ended");
    }
  }
  if(s == -1){
    //if(count > 10){
    vel_msg.linear.x = 0;
    vel_msg.angular.z = 0;
    //}
    aboveAngle = false;
    aboveLength = false;
    for(int i = 0; i < 6; i++){
      if(msg.bodies[i].isTracked == 1){
        s = i;
        ROS_INFO_NAMED("personFollower", "personFollower: tracking new person");
        break;
      }
    }
  }
  // Set the velocities
  if(s != -1){
    //count = 0;
    // Set the angular velocity
    //ROS_INFO_STREAM(msg.bodies[s].jointPositions[0].position.z);
    if(msg.bodies[s].jointPositions[0].position.x >= 0.2){
      vel_msg.angular.z = 0.18;
      aboveAngle = true;
    } 
    else if(msg.bodies[s].jointPositions[0].position.x <= -0.2){
      vel_msg.angular.z = -0.18;
      aboveAngle = true;
    }
    else{
      vel_msg.angular.z = 0;	
      aboveAngle = false;
      //personPos_pub.publish(vel_msg);
    }
    // Set the forward velocity
    if(msg.bodies[s].jointPositions[0].position.z >= 1.5){
      vel_msg.linear.x = 0.18; //msg.bodies[s].jointPositions[3].position.z/7;
      aboveLength = true;
      underLenCount = 0;
    }
    else if(msg.bodies[s].jointPositions[0].position.z < 1.4){
      ++underLenCount;
      if(underLenCount > 3){
        vel_msg.linear.x = 0;
        aboveLength = false;
        underLenCount = 0;
      }
      //personPos_pub.publish(vel_msg);
    }
  }

  // Publish if there is a new state
  ROS_INFO_STREAM(prevAboveAngle);
  ROS_INFO_STREAM(aboveAngle);
  if(prevAboveAngle != aboveAngle || prevAboveLength != aboveLength || s == -1){
    ROS_INFO_STREAM("Publishing message");
    personPos_pub.publish(vel_msg);
  }
  prevAboveAngle = aboveAngle;
  prevAboveLength = aboveLength;
}


int main(int argc,char **argv){
  ros::init(argc,argv,"personFollower");
  ros::NodeHandle n;

  prev_time = ros::Time::now();

  // Set up watchdog
  signal(SIGALRM, watchdog);
  alarm(3);

  // Set up velocity command publisher
  personPos_pub = n.advertise<geometry_msgs::Twist>("RosAria/cmd_vel",1);

  // Subsribe to topic "bodyArray" published by k2_klient package node startBody.cpp
  ros::Subscriber bodies_sub = n.subscribe("head/kinect2/bodyArray", 1, bodies_sub_cb); 
  ros::Subscriber pose_sub = n.subscribe("RosAria/pose", 1, pose_sub_cb); 

  ros::Rate loop_rate(6); //0.1

  ROS_INFO_NAMED("personFollower", "personFollower: Running ROS node...");
  while (ros::ok()){
    ros::spinOnce();
    loop_rate.sleep();
  }
  //ros::spin();

  ROS_INFO_NAMED("personFollower",  "personFollower: Quitting... \n" );

  return 0;
}   
