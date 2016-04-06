//#include <math.h>

#include "ros/ros.h"
#include <k2_client/k2_client.h>
#include <k2_client/BodyArray.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>
#include <deque>


int start_wave[6] = {2,2,2,2,2,2};
int count_wave[6] = {0,0,0,0,0,0};
int lastTime_wave1[6] = {0,0,0,0,0,0};
int lastTime_wave2[6] = {0,0,0,0,0,0};
int time_wave[6] = {0,0,0,0,0,0};

int s=-1;

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
    
    time_wave[bodyNumber] ++;
    if (time_wave[bodyNumber]==1) {
        if(s != -1 && gestureTestMidle(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x) == false && gestureTestAbove(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
            ROS_INFO("Startet");
            if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                start_wave[bodyNumber] = 1;
            }
            else{
                start_wave[bodyNumber] = 0;
            }
            lastTime_wave1[bodyNumber]=time_wave[bodyNumber];
            count_wave[bodyNumber]++;
        }
    }
    else{
        if(s != -1 && gestureTestAbove(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
            if(gestureTestMidle(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x) == false){
                if(start_wave[bodyNumber] == 2){
                    ROS_INFO("Startet");
                    if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                        start_wave[bodyNumber] = 1;
                    }
                    else{
                        start_wave[bodyNumber] = 0;
                    }
                    lastTime_wave1[bodyNumber]=time_wave[bodyNumber];
                    count_wave[bodyNumber]++;
                    ROS_INFO("Fant 1");
                }
                else if(start_wave[bodyNumber] == 0){
                    if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                        start_wave[bodyNumber] = 1;
                        lastTime_wave2[bodyNumber]=time_wave[bodyNumber];
                        count_wave[bodyNumber]++;
                        ROS_INFO("Fant 1");
                    }
                }
                else{
                    if(gestureTestRight(elbowRight.position.y, elbowRight.position.x, handRight.position.y, handRight.position.x)){
                        start_wave[bodyNumber] = 0;
                        lastTime_wave2[bodyNumber]=time_wave[bodyNumber];
                        count_wave[bodyNumber] ++;
                        ROS_INFO("Fant 1");
                    }
                }
            }
        }
    }
    if(count_wave[bodyNumber]==0){
        time_wave[bodyNumber] = 0;
    }
    if(time_wave[bodyNumber] > 60 && count_wave[bodyNumber] < 3){
        if (count_wave[bodyNumber] == 2) {
            time_wave[bodyNumber] -=(lastTime_wave2[bodyNumber]-1);
            lastTime_wave2[bodyNumber] = 0;
            lastTime_wave1[bodyNumber] = 1;
        }
        else {
            lastTime_wave1[bodyNumber] = 0;
            time_wave[bodyNumber] = 0;
            start_wave[bodyNumber]=2;
        }
        if (count_wave[bodyNumber] > 0) {
            count_wave[bodyNumber] --;
        }
    }
}


bool gestureStop(const k2_client::Body& body){
    const auto& shoulderRight = body.jointPositions[8];
    const auto& handRight = body.jointPositions[11];
    
    if(handRight.position.z < (shoulderRight.position.z-0.2) && body.handRightState==2){
        ROS_INFO("Leter etter stop gesture");
        return true;
    }
    else{
        return false;
    }
}



namespace Constants {
    int bodyArrayHistoryMaxSize = 60;
    int bodyArrayNewMaxSize = 10;
}

std::deque<k2_client::BodyArray> bodyArrayHistory;

std::deque<k2_client::BodyArray> bodyArrayNew;


// Subscriber callback function for recieving BodyArray msg from k2_client
void gesture_sub_cb(const k2_client::BodyArray msg){
    //ROS_INFO_NAMED("personGesture", "personGesture: Received bodyArray");
    
    bodyArrayNew.push_back(msg);
    if (bodyArrayNew.size() >= Constants::bodyArrayNewMaxSize){
        bodyArrayNew.pop_back();
    }
    
}


int main(int argc,char **argv){
    ros::init(argc,argv,"personGesture");
    ros::NodeHandle n;
    
    // Subsribe to topic "bodyArray" published by k2_klient package node startBody.cpp
    ros::Subscriber gesture_sub = n.subscribe("head/kinect2/bodyArray", 1000, gesture_sub_cb);
    
    ros::Rate loop_rate(15); //0.1
    
    // Run the ROS node
    ROS_INFO_NAMED("personGesture", "personGesture: Running ROS node...");
    
    bool alive = false;
    int state = 0;
    int afk = 0;
    int count_stop = 0;
    
    while (ros::ok()){
        ros::spinOnce();
        loop_rate.sleep();
        if(alive){
            for(const auto& bodyArray : bodyArrayNew){
                for(int i = 0; i <= 6; i++){
                    s = -1;
                    if(bodyArray.bodies[i].isTracked){
                        s = i;
                    }
                    gestureWave(bodyArray.bodies[i],i);
                    if (count_wave[i] >2) {
                        state = 1;
                        alive = false;
                        ROS_INFO("Wave funnet");
                        for (int j=0; j <= 6; j++) {
                            start_wave[j] = 2;
                            count_wave[j] = 0;
                            lastTime_wave1[j]=0;
                            lastTime_wave2[j]=0;
                            time_wave[j] = 0;
                        }
                        break;
                    }
                }
                if (!alive) {
                    break;
                }
                bodyArrayHistory.push_front(bodyArray);
            }
            bodyArrayNew.clear();
            if (!alive) {
                bodyArrayHistory.clear();
            }
        }
        else{
            for(const auto& bodyArray : bodyArrayNew){
                for(int i = 0; i <= 6; i++){
                    if(bodyArray.bodies[i].isTracked){
                        s = i;
                        break;
                    }
                }
                if(bodyArray.bodies[s].isTracked){
                    afk=0;
                    if(gestureStop(bodyArray.bodies[s])){
                        count_stop++;
                    }
                }
                else{
                    afk++;
                }
                bodyArrayHistory.push_front(bodyArray);
                const auto& historyBack = bodyArrayHistory.back();
                if(bodyArrayHistory.size() >= Constants::bodyArrayHistoryMaxSize && count_stop < 30){
                    if(gestureStop(historyBack.bodies[s]) && count_stop > 0){
                        ROS_INFO("Fail?");
                        count_stop--;
                        ROS_INFO("Nope");
                    }
                    bodyArrayHistory.pop_back();
                }
                if (count_stop >= 30 || afk > 90) {
                    state = 0;
                    alive = true;
                    count_stop = 0;
                    afk=0;
                    bodyArrayHistory.clear();
                    ROS_INFO("Follow stopped");
                    break;
                }
            }
            bodyArrayNew.clear();
        }
        ROS_INFO_STREAM(bodyArrayHistory.size());
        
        if (bodyArrayHistory.size() >= Constants::bodyArrayHistoryMaxSize){
            ROS_INFO("Fail?");
            bodyArrayHistory.pop_back();
            ROS_INFO("Nope");
        }
    }
    ROS_INFO_NAMED("personGesture",  "personGesture: Quitting... \n" );
    return 0;
}
