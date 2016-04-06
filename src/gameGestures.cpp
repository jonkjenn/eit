bool gestureLiftRightFoot(cinst k2_client::Body& body){
    const auto& kneeRight = body.jointPositions[17];
    const auto& kneeLeft = body.jointPositions[13];
    
    //Sjekker om høyre kne er høyere enn venstre kne
    if(kneeright.position.y > ((kneeLeft.position.y)+0.1) ){
        return true;
    }
    
    else{
        return false;
    }
}

bool gesturFlexnes(cinst k2_client::Body& body){
    const auto& elbowRight = body.jointPositions[9];
    const auto& elbowLeft = body.jointPositions[5];
    const auto& shoulderRight = body.jointPositions[8];
    const auto& shoulderLeft = body.jointPositions[4];
    const auto& wristRight = body.jointPositions[10];
    const auto& wristLeft  = body.jointPositions[6];
    
    
    if( (abs(elbowRight.position.y - shoulderRight.position.y)<0.1) && (abs(wristRight.position.x == elbowRight.position.x)<0.1) && (abs(elbowLeft.position.y == shoulderLeft.position.y)<0.1) && (abs(wristLeft.position.x == elbowLeft.position.x)<0.1)) {
        return true;
    }
    else{
        return false;
    }
    
    
}
