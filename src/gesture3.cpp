//#include <math.h>

#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>
#include <deque>

int count[6] = {0,0,0,0,0,0};
int start_wave[6] = {2,2,2,2,2,2};
int lastTime_wave1[6] = {0,0,0,0,0,0};
int lastTime_wave2[6] = {0,0,0,0,0,0};
int time_count[6] = {0,0,0,0,0,0};

namespace Constants {
   unsigned int bodyArrayHistoryMaxSize = 60;
   unsigned int bodyArrayNewMaxSize = 10;
}

//std::deque<k2_client::BodyArray> bodyArrayHistory;

struct boolArray{
    bool data[6];
};

std::deque<boolArray> bodyArrayHistory;

std::deque<k2_client::BodyArray> bodyArrayNew;


// Subscriber callback function for recieving BodyArray msg from k2_client
void gesture_sub_cb(const k2_client::BodyArray msg){
    //ROS_INFO_NAMED("personGesture", "personGesture: Received bodyArray");
    
    bodyArrayNew.push_back(msg);
    if (bodyArrayNew.size() >= Constants::bodyArrayNewMaxSize){
        bodyArrayNew.pop_back();
    }
    
}

bool gestureTestMidle(double y1, double x1, double y2, double x2){
    if(abs(x1 - x2) <= 0.05*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
        return true;
    }
    else{
        return false;
    }
}

bool gestureTestRight(double y1, double x1, double y2, double x2){
    if(x1 < x2 && abs(x1 - x2) > 0.05*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
        return true;
    }
    else{
        return false;
    }
}

bool gestureTestAbove(double y1, double x1, double y2, double x2){
    if(y1 > y2 && abs(y1 - y2) > 0.05*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
        return true;
    }
    else{
        return false;
    }
}




void gestureWave(const k2_client::Body& body, int bodyNumber){
    
    const auto& elbowRight = body.jointPositions[9];
    const auto& handRight = body.jointPositions[11];
    
    time_count[bodyNumber] ++;
    
    if(body.isTracked){
        if (time_count[bodyNumber]==1) {
            if(gestureTestMidle(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x) == false && gestureTestAbove(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                    start_wave[bodyNumber] = 1;
                }
                else{
                    start_wave[bodyNumber] = 0;
                }
                lastTime_wave1[bodyNumber]=time_count[bodyNumber];
                count[bodyNumber]++;
                ROS_INFO("Fant 1");
            }
        }
        else{
            if(gestureTestAbove(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                if(gestureTestMidle(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x) == false){
                    if(start_wave[bodyNumber] == 2){
                        if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                            start_wave[bodyNumber] = 1;
                        }
                        else{
                            start_wave[bodyNumber] = 0;
                        }
                        lastTime_wave1[bodyNumber]=time_count[bodyNumber];
                        count[bodyNumber]++;
                        ROS_INFO("Fant 1");
                    }
                    else if(start_wave[bodyNumber] == 0){
                        if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                            start_wave[bodyNumber] = 1;
                            lastTime_wave2[bodyNumber]=time_count[bodyNumber];
                            count[bodyNumber]++;
                            ROS_INFO("Fant 1");
                        }
                    }
                    else{
                        if(gestureTestRight(elbowRight.position.y, elbowRight.position.x, handRight.position.y, handRight.position.x)){
                            start_wave[bodyNumber] = 0;
                            lastTime_wave2[bodyNumber]=time_count[bodyNumber];
                            count[bodyNumber] ++;
                            ROS_INFO("Fant 1");
                        }
                    }
                }
            }
        }
    }
    if(count[bodyNumber]==0){
        time_count[bodyNumber] = 0;
    }
    else if(time_count[bodyNumber] > 60 && count[bodyNumber] < 3){
        if (count[bodyNumber] == 2) {
            time_count[bodyNumber] -=(lastTime_wave2[bodyNumber]-1);
            lastTime_wave2[bodyNumber] = 0;
            lastTime_wave1[bodyNumber] = 1;
        }
        else {
            lastTime_wave1[bodyNumber] = 0;
            time_count[bodyNumber] = 0;
            start_wave[bodyNumber]=2;
        }
        count[bodyNumber] --;
    }
}





bool gesture_start(){
    bool ret = false;
    for(const auto& bodyArray : bodyArrayNew){
        for(int i = 0; i < 6; i++){
            gestureWave(bodyArray.bodies[i],i);
            if (count[i] >2) {
                ret = true;
                ROS_INFO("Wave funnet");
                for (int j=0; j < 6; j++) {
                    start_wave[j] = 2;
                    count[j] = 0;
                    lastTime_wave1[j]=0;
                    lastTime_wave2[j]=0;
                    time_count[j] = 0;
                }
                break;
            }
        }
        if (ret) {
            bodyArrayHistory.clear();
            break;
        }
    }
    bodyArrayNew.clear();
    return ret;
}




bool gestureStop(const k2_client::Body& body){
    const auto& shoulderRight = body.jointPositions[8];
    const auto& handRight = body.jointPositions[11];
    
    if(handRight.position.z < (shoulderRight.position.z-0.2) && body.handRightState==2){
        ROS_INFO("Hand");
        return true;
    }
    else{
        return false;
    }
}


bool gestureLiftRightFoot(const k2_client::Body& body){
    const auto& kneeRight = body.jointPositions[17];
    const auto& kneeLeft = body.jointPositions[13];
    
    //Sjekker om høyre kne er høyere enn venstre kne
    if(kneeRight.position.y > ((kneeLeft.position.y)+0.1) ){
        return true;
    }
    else{
        return false;
    }
}

bool gestureFlexnes(const k2_client::Body& body){
    const auto& elbowRight = body.jointPositions[9];
    const auto& elbowLeft = body.jointPositions[5];
    const auto& shoulderRight = body.jointPositions[8];
    const auto& shoulderLeft = body.jointPositions[4];
    const auto& wristRight = body.jointPositions[10];
    const auto& wristLeft  = body.jointPositions[6];
    
    
    if( (abs(elbowRight.position.y - shoulderRight.position.y)<0.1) && (abs(wristRight.position.x - elbowRight.position.x)<0.1) && (abs(elbowLeft.position.y - shoulderLeft.position.y)<0.1) && (abs(wristLeft.position.x - elbowLeft.position.x)<0.1)) {
        return true;
    }
    else{
        return false;
    }
}




bool gestureXrossLegs(const k2_client::Body& body){
    const auto& ankleRight = body.jointPositions[18];
    const auto& ankleLeft = body.jointPositions[14];

    if(ankleRight.position.x > ankleLeft.position.x or ankleLeft.position.x < ankleRight.position.x){
        return true;

    }
    else {
        return false;
    }
}

bool gestureHandsStraightUp(const k2_client::Body& body){
    const auto& elbowLeft = body.jointPositions[5]; 
    const auto& elbowRight = body.jointPositions[9];
    const auto& head = body.jointPositions[3];
    const auto& shoulderRight = body.jointPositions[8];
    const auto& shoulderLeft = body.jointPositions[4];

    if(  elbowRight.position.y > head.position.y && elbowLeft.position.y > head.position.y && abs(shoulderRight.position.x - elbowRight.position.x)<0.1 && abs(shoulderLeft.position.x - elbowLeft.position.x)<0.1                         ){
        return true;
    }
    else{
        return false;
    }

}


bool gestureStandOnToes(const k2_client::Body& body){
    const auto& footRight = body.jointPositions[19];
    const auto& footLeft = body.jointPositions[15];
    const auto& ankleRight = body.jointPositions[18];
    const auto& ankleLeft = body.jointPositions[14];


   if ( (ankleRight.position.y - footRight.position.y)>0.05 && (ankleLeft.position.y - footLeft.position.y)>0.05){
        return true;
   }
   else{
        return false; 
   }
}


bool gestures(const k2_client::Body& body, int b){
    switch(b){
        case 1:
            return gestureStop(body);
            break;
         case 2:
            return gestureLiftRightFoot(body);
            break;
         case 3:
            return gestureStandOnToes(body);
            break;
        default :
            return false;
    }
    
    
}






int gestureCall(int b){
    int afk_count=0;
    bool success = false;
    for(const auto& bodyArray : bodyArrayNew){
        boolArray temp;
        for(int i = 0; i < 6; i++){
            temp.data[i]=false;
            if(bodyArray.bodies[i].isTracked){
                time_count[i]=0;
                if(gestures(bodyArray.bodies[i],b)){
                    ROS_INFO_STREAM(count[i]);
                    count[i]++;
                    temp.data[i]=true;
                }
            }
            else{
                time_count[i]++;
            }
        }
        bodyArrayHistory.push_front(temp);
        
        const auto& historyBack = bodyArrayHistory.back();
        if(bodyArrayHistory.size() >= Constants::bodyArrayHistoryMaxSize){
            for(int i = 0; i < 6; i++){
                if(count[i] > 0 && count[i] < 30 && historyBack.data[i]){
                   ROS_INFO_STREAM(count[i]);
                    count[i]--;
                }
            }
            bodyArrayHistory.pop_back();
        }
        afk_count = 0;
        for(int i = 0; i < 6; i++){
            if (time_count[i] > 120) {
                afk_count++;
            }
            if (count[i] >= 30){
                success =true;
                break;
            }
        }
        if (success || afk_count >= 6){
            bodyArrayHistory.clear();
            for (int j=0; j < 6; j++) {
                count[j] = 0;
                time_count[j] = 0;
            }
            break;
        }
    }
    bodyArrayNew.clear();
    if (success){
        ROS_INFO("Success");
        return 1;
    }
    else if(afk_count >= 6){
        ROS_INFO("Afk");
        return 0;
    }
    else{
        return 2;
    }
    
}


int main(int argc,char **argv){
    ros::init(argc,argv,"personGesture");
    ros::NodeHandle n;
    
    // Subsribe to topic "bodyArray" published by k2_klient package node startBody.cpp
    ros::Subscriber gesture_sub = n.subscribe("head/kinect2/bodyArray", 1000, gesture_sub_cb);
    
    ros::Rate loop_rate(3); //0.1
    
    // Run the ROS node
    ROS_INFO_NAMED("personGesture", "personGesture: Running ROS node...");
    
    bool state = true;
    
    while (ros::ok()){
        ros::spinOnce();
        loop_rate.sleep();
        if (state) {
            int b = 3;
            if(gestureCall(b)!= 2){
                state = false;
            }
        }
        else{
            state = gesture_start();
        }
    }
    ROS_INFO_NAMED("personGesture",  "personGesture: Quitting... \n" );
    return 0;
}

