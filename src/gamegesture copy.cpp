#include "cyborg_simon_says/gestures.h"

#include "ros/ros.h"
//#include <geometry_msgs/Twist.h>
//#include <tf/transform_listener.h>

#include <deque>

int count[6] = {0,0,0,0,0,0};
int start_wave[6] = {2,2,2,2,2,2};
int lastTime_wave[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int time_count[6] = {0,0,0,0,0,0};

namespace Constants {
    unsigned int bodyArrayHistoryMaxSize = 50;
    unsigned int bodyArrayNewMaxSize = 10;
}

//std::deque<k2_client::BodyArray> bodyArrayHistory;

struct boolArray{
    bool data[6];
};

std::deque<boolArray> bodyArrayHistory;

std::deque<k2_client::BodyArray> bodyArrayNew;


// Subscriber callback function for recieving BodyArray msg from k2_client
void
cyborg::simon_says::gestures::bodyArrayMessageHandler(const k2_client::BodyArray msg){
    //ROS_INFO_NAMED("personGesture", "personGesture: Received bodyArray");
    
    bodyArrayNew.push_back(msg);
    if (bodyArrayNew.size() >= Constants::bodyArrayNewMaxSize){
        bodyArrayNew.pop_back();
    }
    
}

bool gestureTestMidle(double y1, double x1, double y2, double x2){
    if(fabs(x1 - x2) <= 0.2*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
        return true;
    }
    else{
        return false;
    }
}

bool gestureTestRight(double y1, double x1, double y2, double x2){
    if(x1 < x2 && fabs(x1 - x2) > 0.2*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
        return true;
    }
    else{
        return false;
    }
}

bool gestureTestAbove(double y1, double x1, double y2, double x2){
    if(y2 < y1 && fabs(y1 - y2) > 0.2*sqrt(pow((y1-y2),2) + pow((x1-x2),2))){
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
        if(gestureTestAbove(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
            if(gestureTestMidle(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x) == false){
                if (time_count[bodyNumber]==1) {
                    if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                        start_wave[bodyNumber] = 1;
                    }
                    else{
                        start_wave[bodyNumber] = 0;
                    }
                    lastTime_wave[bodyNumber]=time_count[bodyNumber];
                    count[bodyNumber]++;
                }
                else{
                    if(start_wave[bodyNumber] == 2){
                        if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                            start_wave[bodyNumber] = 1;
                        }
                        else{
                            start_wave[bodyNumber] = 0;
                        }
                        lastTime_wave[bodyNumber]=time_count[bodyNumber];
                        count[bodyNumber]++;
                    }
                    else if(start_wave[bodyNumber] == 0){
                        if(gestureTestRight(handRight.position.y, handRight.position.x, elbowRight.position.y, elbowRight.position.x)){
                            start_wave[bodyNumber] = 1;
                            lastTime_wave[bodyNumber+6*count[bodyNumber]]=time_count[bodyNumber];
                            count[bodyNumber]++;
                        }
                    }
                    else{
                        if(gestureTestRight(elbowRight.position.y, elbowRight.position.x, handRight.position.y, handRight.position.x)){
                            start_wave[bodyNumber] = 0;
                            lastTime_wave[bodyNumber+6*count[bodyNumber]]=time_count[bodyNumber];
                            count[bodyNumber] ++;
                        }
                    }
                }
            }
        }
        //ROS_INFO_STREAM(count[bodyNumber]);
    }
    if(count[bodyNumber]==0){
        time_count[bodyNumber] = 0;
    }
    else if(time_count[bodyNumber] > 60 && count[bodyNumber] < 4){
        if (count[bodyNumber] == 3) {
            time_count[bodyNumber] -=(lastTime_wave[bodyNumber+6]-1);
            lastTime_wave[bodyNumber+6]=lastTime_wave[bodyNumber+6*2]-1;
            lastTime_wave[bodyNumber+6*2] = 0;
            lastTime_wave[bodyNumber] = 1;
        }
        else if (count[bodyNumber] == 2) {
            time_count[bodyNumber] -=(lastTime_wave[bodyNumber+6]-1);
            lastTime_wave[bodyNumber+6] = 0;
            lastTime_wave[bodyNumber] = 1;
        }
        else {
            lastTime_wave[bodyNumber] = 0;
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
                    lastTime_wave[j]=0;
                    lastTime_wave[j+6]=0;
                    lastTime_wave[j+12]=0;
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

bool gestureLiftLeftFoot(const k2_client::Body& body){
    const auto& kneeRight = body.jointPositions[17];
    const auto& kneeLeft = body.jointPositions[13];
    
    //Sjekker om høyre kne er høyere enn venstre kne
    if(kneeLeft.position.y > ((kneeRight.position.y)+0.1) ){
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
    
    
    if( (fabs(elbowRight.position.y - shoulderRight.position.y)<0.1) && (fabs(wristRight.position.x - elbowRight.position.x)<0.1) && (fabs(elbowLeft.position.y - shoulderLeft.position.y)<0.1) && (fabs(wristLeft.position.x - elbowLeft.position.x)<0.1)) {
        return true;
    }
    else{
        return false;
    }
}


bool gestureHandsStraightUp(const k2_client::Body& body){
    const auto& elbowLeft = body.jointPositions[5];
    const auto& elbowRight = body.jointPositions[9];
    const auto& head = body.jointPositions[3];
    const auto& shoulderRight = body.jointPositions[8];
    const auto& shoulderLeft = body.jointPositions[4];
    const auto& wristRight = body.jointPositions[10];
    const auto& wristLeft  = body.jointPositions[6];
    
    if(  wristRight.position.y > head.position.y && wristLeft.position.y > head.position.y && fabs(shoulderRight.position.x - elbowRight.position.x)<0.2 && fabs(shoulderLeft.position.x - elbowLeft.position.x)<0.2){
        return true;
    }
    else{
        return false;
    }
    
}


bool gestureTouchRightFoot(const k2_client::Body& body){
    const auto& footRight = body.jointPositions[19];
    const auto& wristRight = body.jointPositions[10];
    const auto& wristLeft  = body.jointPositions[6];
    
    
    if ( (fabs(wristRight.position.y - footRight.position.y)<0.2 && fabs(wristRight.position.x - footRight.position.x)<0.2) || (fabs(wristLeft.position.x - footRight.position.x)<0.2 && fabs(wristLeft.position.y - footRight.position.y)<0.2){
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
            return gestureLiftLeftFoot(body);
            break;
        case 4:
            return gestureFlexnes(body);
            break;
        case 5:
            return gestureHandsStraightUp(body);
            break;
        case 5:
            return gestureTouchRightFoot(body);
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
                    //ROS_INFO_STREAM(count[i]);
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
                    //ROS_INFO_STREAM(count[i]);
                    count[i]--;
                }
            }
            bodyArrayHistory.pop_back();
        }
        afk_count = 0;
        for(int i = 0; i < 6; i++){
            if (time_count[i] > 60) {
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

cyborg::simon_says::GestureEvent::GestureEvent(const Gesture gesture)
: gesture_(gesture)
{
}

std::string
cyborg::simon_says::GestureEvent::announcement()
{
    switch (gesture_)
    {
        case Gesture::liftRightFoot:
            return "Raise your right foot!";
        case Gesture::liftLeftFoot:
            return "Raise your left foot!";
        case Gesture::handInFront:
            return "Put your open right hand infront of you!";
        case Gesture::flexnes:
            return "Flex your muscles!";
        case Gesture::handsStraightUp:
            return "Put your hands up!";
        case Gesture::TouchRightFoot:
            return "Touch your right foot!";
        default:
            return "";
    }
}

bool
cyborg::simon_says::GestureEvent::completed()
{
    if (gesture_ == Gesture::wave)
    {
        return gesture_start();
    }
    else
    {
        return gestureCall(static_cast<int>(gesture_)) == 1;
    }
}

void
cyborg::simon_says::GestureEvent::start()
{
    // Clear gesture state
}

double
cyborg::simon_says::GestureEvent::timeout()
{
    // Timeout period in seconds
    return 20.0;
}
