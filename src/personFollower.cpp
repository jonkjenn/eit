#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/LaserScan.h>
#include <rosaria/BumperState.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_listener.h>
#include <signal.h>
#include <string>
#include <cyborg_coordinator/RequestControl.h>
#include <cyborg_coordinator/ReleaseControl.h>
//#include "pid.h"

using namespace std;

ros::Publisher personPos_pub;
geometry_msgs::Twist vel_msg;
geometry_msgs::Twist prev_vel_msg;

string my_identifier = "personFollower";

ros::Time prev_time;
float prev_vel_z;

int checkCount = 0;
int s = -1;
bool aboveAngle = false;
bool aboveLength = false;
bool prevAboveAngle = false;
bool prevAboveLength = false;
int lr = 6;

bool stopped = false;

bool use_coordinator = true; //Should coordinator be used?
bool has_control = false; //Has personFollower been given control?

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

bool gestureTestMidle(const k2_client::Body& body){
	const auto& shoulderRight = body.jointPositions[8];
	const auto& handRight = body.jointPositions[11];
	const auto& shoulderLeft = body.jointPositions[4];
	const auto& handLeft = body.jointPositions[7];

	if(abs(shoulderRight.position.y - handRight.position.y) <= 0.2 || abs(shoulderLeft.position.y - handLeft.position.y) <= 0.2){
		return true;
	}
	else{
		return false;
	}
}

void laser_sub_cb(const sensor_msgs::LaserScan msg){
}

void bumper_state_sub_cb(const rosaria::BumperState msg){ //check if bumper is pushed in
	for(bool b:msg.front_bumpers){
		if(b){
			ROS_INFO_STREAM("Stopping: ");
			stopped = true;
			return;
		}
	}
	ROS_INFO_STREAM("Not stopped ");
	stopped = false;
}

void bodies_sub_cb(const k2_client::BodyArray msg){
	alarm(1);
	vel_msg.linear.y = 0; 
	vel_msg.linear.z = 0; 
	vel_msg.angular.x = 0;
	vel_msg.angular.y = 0;

	//if((false == use_coordinator || has_control) && stopped){
	ROS_INFO_STREAM("Stopped? " << stopped);
	if(stopped){
		vel_msg.linear.x = 0;
		vel_msg.angular.z = 0;
		personPos_pub.publish(vel_msg);
		cyborg_coordinator::ReleaseControl rel;
		rel.request.id = my_identifier;
		ros::service::call("/cyborg_coordinator/releaseControl", rel);
		has_control = false;
		return;
	}

	// Continue to track the same person, or find a new person to track (if any)

	if(s == -1){
		vel_msg.linear.x = 0;
		vel_msg.angular.z = 0;
		aboveAngle = false;
		aboveLength = false;
		for(int i = 0; i < 6; i++){
			if(msg.bodies[i].isTracked && gestureTestMidle(msg.bodies[i])){
				s = i;
				break;
			}
		}
	}

	if(s != -1){
		bool found = false;
		for(int i = 0; i < 6; i++){
			if(msg.bodies[i].isTracked && gestureTestMidle(msg.bodies[i])){
				found = true;
				break;
			}
		}
		if(!found && checkCount==0){
			s = -1;
			checkCount = 1;
		}
	}

	// Set the velocities
	if(s != -1){
		if(use_coordinator == true && false == has_control) {
			cyborg_coordinator::RequestControl req;
			req.request.id = my_identifier;
			ros::service::call("/cyborg_coordinator/requestControl", req); //ask for control
			has_control = req.response.controlReceived;
		}
		if(has_control == true || use_coordinator == false){
			// Set the angular velocity
			if(msg.bodies[s].jointPositions[0].position.x >= 0.2){
				vel_msg.angular.z = 0.20;
				aboveAngle = true;
			} 
			else if(msg.bodies[s].jointPositions[0].position.x <= -0.2){
				vel_msg.angular.z = -0.20;
				aboveAngle = true;
			}
			else{
				vel_msg.angular.z = 0;	
				aboveAngle = false;
			}
			// Set the forward velocity

			if(msg.bodies[s].jointPositions[0].position.z >= 2.0){
				vel_msg.linear.x = 0.20; //msg.bodies[s].jointPositions[3].position.z/7;
				aboveLength = true;
				underLenCount = 0;
			}
			else if(msg.bodies[s].jointPositions[0].position.z < 2.0){
				++underLenCount;
				if(underLenCount > 3){
					vel_msg.linear.x = 0;
					aboveLength = false;
					underLenCount = 0;
				}
			}
		}
	}

	//Check to see if some time threshold has been passed, if not, keep going
	if(checkCount > 0 && checkCount < 2*lr){ 
		checkCount++;
		//ROS_INFO_STREAM("checkCount: " << checkCount);
		vel_msg = prev_vel_msg;
	}
	else if(checkCount >= 2*lr){
		checkCount = 0;
		if(has_control == true){
			vel_msg.linear.x = 0;
			vel_msg.angular.z = 0;
			personPos_pub.publish(vel_msg);
			cyborg_coordinator::ReleaseControl rel;
			rel.request.id = my_identifier;
			ros::service::call("/cyborg_coordinator/releaseControl", rel);
			has_control = false;
		}
	}

	//ROS_INFO_STREAM(std::boolalpha << "prevAboveAngle " << prevAboveAngle << " aboveAngle " << aboveAngle << " prevAboveLength " << prevAboveLength << " aboveLength " << aboveLength);

	// Publish if there is a new state
	//if(prevAboveAngle != aboveAngle || prevAboveLength != aboveLength || s == -1){
	//ROS_INFO_STREAM("Publishing ");
	prev_vel_msg = vel_msg;
	personPos_pub.publish(vel_msg);

	prevAboveAngle = aboveAngle;
	prevAboveLength = aboveLength;
}


int main(int argc,char **argv){
	ros::init(argc,argv,"personFollower");
	ros::NodeHandle n;
	n.getParam("use_coordinator",use_coordinator);

	prev_time = ros::Time::now();

	// Set up watchdog
	signal(SIGALRM, watchdog);
	alarm(3);

	if(use_coordinator)
	{
		personPos_pub = n.advertise<geometry_msgs::Twist>("cyborg_coordinator/"+my_identifier+"/RosAria/cmd_vel",1);
	}
	else{
		// Set up velocity command publisher
		personPos_pub = n.advertise<geometry_msgs::Twist>("RosAria/cmd_vel",1);
	}


	// Subsribe to topic "bodyArray" published by k2_klient package node startBody.cpp
	ros::Subscriber bodies_sub = n.subscribe("head/kinect2/bodyArray", 1, bodies_sub_cb); 
	ros::Subscriber laser_sub = n.subscribe("RosAria/S3Series_1_laserscan", 1, laser_sub_cb); 
	ros::Subscriber bumper_sub = n.subscribe("RosAria/bumper_state", 1, bumper_state_sub_cb);

	ros::Rate loop_rate(lr); //0.1

	while (ros::ok()){
		ros::spinOnce();
		loop_rate.sleep();
	}
	//ROS_INFO_NAMED("personFollower",  "personFollower: Quitting... \n" );

	return 0;
}   
