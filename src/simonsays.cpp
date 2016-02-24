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
   
    while(ros::ok())
    {
        k2_client::BodyArray bodyArray;
        try
        {
            for(int i=0;i<1;i++)
            {
                k2_client::Body body;
                double utcTime = true;
                body.header.stamp = ros::Time(utcTime);
                body.header.frame_id =  ros::this_node::getNamespace().substr(1,std::string::npos) + "/depthFrame";
                body.leanTrackingState = 0;
                body.lean.leanX = 0;
                body.lean.leanY = 0;
                body.isTracked = true;
                body.trackingId = 0;
                body.clippedEdges = 0;
                body.engaged = false;
                body.handRightConfidence = 0;
                body.handRightState = 0;
                body.handLeftConfidence = 0;
                body.handLeftState = 0;
                body.appearance.wearingGlasses = true;
                body.activities.eyeLeftClosed = false;
                body.activities.eyeRightClosed = false;
                body.activities.mouthOpen = false;
                body.activities.mouthMoved = false;
                body.activities.lookingAway = false;
                body.expressions.neutral = false;
                body.expressions.neutral = false;
                for(int j=0;j<25;j++)
                {
                    k2_client::JointOrientationAndType JOAT;
                    k2_client::JointPositionAndState JPAS;
                    std::string fieldName;
                    switch (j)
                    {
                        case 0: fieldName = "SpineBase";break;
                        case 1: fieldName = "SpineMid";break;
                        case 2: fieldName = "Neck";break;
                        case 3: fieldName = "Head";break;
                        case 4: fieldName = "ShoulderLeft";break;
                        case 5: fieldName = "ElbowLeft";break;
                        case 6: fieldName = "WristLeft";break;
                        case 7: fieldName = "HandLeft";break;
                        case 8: fieldName = "ShoulderRight";break;
                        case 9: fieldName = "ElbowRight";break;
                        case 10: fieldName = "WristRight";break;
                        case 11: fieldName = "HandRight";break;
                        case 12: fieldName = "HipLeft";break;
                        case 13: fieldName = "KneeLeft";break;
                        case 14: fieldName = "AnkleLeft";break;
                        case 15: fieldName = "FootLeft";break;
                        case 16: fieldName = "HipRight";break;
                        case 17: fieldName = "KneeRight";break;
                        case 18: fieldName = "AnkleRight";break;
                        case 19: fieldName = "FootRight";break;
                        case 20: fieldName = "SpineShoulder";break;
                        case 21: fieldName = "HandTipLeft";break;
                        case 22: fieldName = "ThumbLeft";break;
                        case 23: fieldName = "HandTipRight";break;
                        case 24: fieldName = "ThumbRight";break;
                    }

                    JOAT.orientation.x = 0;
                    JOAT.orientation.y = 0;
                    JOAT.orientation.z = 0;
                    JOAT.orientation.w = 0;
                    JOAT.jointType = 0;

                    JPAS.trackingState = true;
                    JPAS.position.x = 0;
                    JPAS.position.y = 0;
                    JPAS.position.z = 0;
                    JPAS.jointType = 0;
                    
                    body.jointOrientations.push_back(JOAT);
                    body.jointPositions.push_back(JPAS);
                }
                body.jointPositions[3].position.x = xpos;
                body.jointPositions[3].position.z = zpos;
                bodyArray.bodies.push_back(body);
            }
        }
        catch (...)
        {
            ROS_ERROR("An exception occured");
            continue;
        }
        bodyPub.publish(bodyArray);
        ros::Duration(1.0).sleep();
        ROS_INFO_NAMED("kinectsimulator", "Published bodyarray");
    }
    return 0;
}
