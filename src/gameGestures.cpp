bool gestureLiftRightFoot(const k2_client::Body& body){
    const auto& kneeRight = body.jointPositions[17];
    const auto& kneeLeft = body.jointPositions[13];
    
    //Sjekker om høyre kne er høyere enn venstre kne
    if(kneeright.position.y > ((kneeLeft.position.y)0.1) ){
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


bool gestureStop(const k2_client::Body:: body){
    const auto& handRight = body.jointPositions[11];
    const audo& shoulderRight = body.jointPositions[8];
    
    if(abs(handRight.position.y - shoulderRight.position.y )<0.1  &&  (handRight.position.z < shoulderRight.position.z)){
        return true;
    }
    else {
        return false;
    }
}


bool gestureXrossLegs(const k2_client::Body:: body){
    const auto& ankleRight = body.jointPositions[18];
    const auto& ankleLeft = body.jointPositions[14];

    if(ankleRight.position.x > ankleLeft.positionx or ankleLeft.position.x < ankleRight.position.x){
        return true;

    }
    else {
        return false;
    }
}

bool gestureHandsStraightUp(const k2_client::Body:: body){
    const auto& elbowLeft = body.jointPosition[5]; 
    const auto& elbowRight = body.jointPosition[9];
    const auto& head = body.jointPosition[3];
    const auto& shoulderRight = body.jointPosition[8];
    const auto& shoulderLeft = body.jointPosition[4];

    if(  elbowRight.position.y > head.position.y && elbowLeft.position.y > head.position.y && abs(shoulderRight.position.x - elbowRight.position.x)<0.1 && abs(shoulderLeft.position.x - elbowLeft.position.x)<0.1                         ){
        return true;
    }
    else{
        return false;
    }

}
