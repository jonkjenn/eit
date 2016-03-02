#include "k2_client/BodyArray.h"
#include "ros/ros.h"
#include <vector>
#include <string>
#include "rosaria/BumperState.h"

std::string topicName = "bodyArray";

void bump(const rosaria::BumperState::ConstPtr &bump){
        ROS_INFO_NAMED("kinectsimulator", "BUMPED");
}


int main(int argC,char **argV)
{

    std::vector<std::string> args;
    for(int i=1;i<argC;++i){
      args.push_back(std::string(argV[i]));
    }

    double xpos;
    double zpos;

    if(args.size() == 2){
      xpos = stod(args[0],nullptr);
      zpos = stod(args[1],nullptr);
    }

    ros::init(argC,argV,"startBody");
    ros::NodeHandle n;
    ros::Publisher bodyPub = n.advertise<k2_client::BodyArray>(topicName,1);
    ros::NodeHandle bu;
    ros::Subscriber bodies_sub = bu.subscribe("RosAria/BumperState", 1, bump);

    ros::Rate loop_rate(20); //0.1
}
