#include "k2_client/BodyArray.h"
#include "ros/ros.h"
#include <vector>
#include <string>
#include "std_msgs/String.h"
#include <stdib.h>
#include <time.h>

namespace JointType
{
         "SpineBase";break;
         "SpineMid";break;
         "Neck";break;
         "Head";break;
         "ShoulderLeft";break;
         "ElbowLeft";break;
         "WristLeft";break;
         "HandLeft";break;
         "ShoulderRight";break;
         "ElbowRight";break;
         "WristRight";break;
         "HandRight";break;
         "HipLeft";break;
         "KneeLeft";break;
         "AnkleLeft";break;
         "FootLeft";break;
         "HipRight";break;
         "KneeRight";break;
         "AnkleRight";break;
         "FootRight";break;
         "SpineShoulder";break;
         "HandTipLeft";break;
         "ThumbLeft";break;
         "HandTipRight";break;
         "ThumbRight";break;
}





void bodies_sub_cb(const k2_client::BodyArray msg)
{
  
    
   //gjenkjenne gesturer fra kinnect:
    
  for (const auto& body : msg.bodies){
      
    //Ser om personen er glad
      
    if (body.expressions.happy){
        
      ROS_INFO("The user eats happymeal ");
        
    }
      
    //Ser om personen er nøytral
      
    else if (body.expressions.neutral){
        
      ROS_INFO("The user is neutral ");
        
    }
    //Ser om personen har øynene lukket
      
    if(body.activities.eyeleftClosed and body.activities.eyeRightClosed){
        
      ROS_INFO("Eye closed af");
        
    }
    //ser om personen har på seg briller
      
    if(body.appearance.wearingGlasses){
        
      ROS_INFO("Wearing Glases");
        
    }
      
  }

}

enum class GameState
{
    NotStarted, InGame
};

enum class Challenge
{
    ArmsInAir
};

std::deque<Challenge> challenges;

void evaluateGameState()
{
    switch (gameState)
    {
        case GameState::NotStarted:
            if (isStartGestureDetected)
            {
                challenges = generateChallenges();
                gameState  = GameState.InGame;
            }
            break;
        case GameState::InGame:
            if (!challenges.empty())
            {
                if (isChallengeCompleted(*challenges.first()))
                {
                    challenges.pop_front();
                }
            }
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "listener");
    ros::NodeHandle n;
    
    ros::Subscriber sub = n.subscribe("head/kinect2/bodyArray", 1000, bodies_sub_cb);
    
    int RandomStateNumber;
    bool gameState;
    int start = 1;
    
    while (true)
    {
        evaluateGameState();
        ros::spin();
    }
    
    while (1){
      srand(time(NULL));
      RandomStateNumber = rand() % 5;
      
      switch(start)
      {
        case 0: break;
        case 1: if (gameState){
          start = 0;
          break;
        }
        case 2: break;
        
      }
    
    
    
    
    
}
  return 0;

}

