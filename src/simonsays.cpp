#include "k2_client/BodyArray.h"
#include "ros/ros.h"
#include <vector>
#include <string>
#include "std_msgs/String.h"
#include <stdib.h>
#include <time.h>

#include <chrono>
#include <deque>

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

enum class Challenge
{
    ArmsInAir
};

enum class Gesture
{
    Start
};

std::deque<Challenge> challenges;


void outputUserMessage(const std::string& message)
{
    std::cout << message << std::endl;
}

void challengeCompleted(const Challenge& challenge)
{
    outputUserMessage(std::string("Challenge '") + challenge.name() + "' completed!");
}

void gameCompleted()
{
    outputUserMessage("Game completed!");
}

class Challenge
{
    public:
        Challenge æ@æø.,-,–…‚–…‚–·„—·„–…‚
};

class Game
{
    public:
        enum class State
        {
            notStarted, beginChallenge, 
        };

        Game(ros::NodeHandle&   node,
             GestureController& gestureController)
            : state_{State::notStarted},
              node_{node},
              gestureController_{gestureController}
        {}

        void evaluate()
        {
            switch (state_)
            {
                case State::notStarted:
                {
                    if (gestureController.hasDetected(GestureController::Gestures::start))
                    {
                        if (requestControl())
                        {
                            challenges_ = generateChallenges();
                            state_      = State::challengeBegin;
                        }
                        else
                        {
                            gestureController_.clear(GestureController::Gestures::start);
                        }
                    }
                    break;
                }
                case State::challengeBegin:
                {
                    if (!challenges_.empty())
                    {
                        auto& currentChallenge = *challenges_.first();
                        
                        startTime_ = std::chrono::steady_clock::now();
                        gestureController_.clearGesture(currentChallenge.gesture);
                        currentChallenge->begin();
                        state_ = State::challengeInProgress;
                    }
                    else
                    {
                        state_ = State::gameCompleted;
                    }
                    break;
                }
                case State::challengeInProgress:
                {
                    auto& currentChallenge = *challenges_.first();
                    const auto currentTime = std::chrono::steady_clock::now();

                    double challengeTimeout;
                    node.param("challenge_timeout", challengeTimeout, 10.0);

                    if (currentTime <= startTime_ + std::chrono::duration<double>{challengeTimeout})
                    {
                        if (challenge->isCompleted())
                        {
                            challengeCompleted(challenge);
                        }
                    }
                    else
                    {
                        challengeTimeout(challenge);
                        startTime_ = currentTime;
                        state_ = State::paused;
                    }

                    break;
                }
                case State::paused:
                {
                    const auto currentTime = std::chrono::steady_clock::now();

                    double gameOverTimeout;
                    node.param("game_over_timeout", gameOverTimeout, 5.0);

                    if (currentTime > startTime_ + gameOverTimeout)
                    {
                        state_ = State::notStarted;
                        challenges_.clear();
                    }

                    break;
                }
            }
        }

    private:
        ros::NodeHandle&   node_;
        GestureController& gestureController_;
        
        State                                              state_;
        std::deque<Challenge>                              challenges_;
        std::chrono::time_point<std::chrono::steady_clock> startTime_;

};

void evaluateGameState(ros::NodeHandle& node, GameState& gameState)
{
    switch (gameState)
    {
        case GameState::notStarted:
            if (isGestureDetected(Gesture::start))
            {
                if (requestControl())
                {
                    challenges = generateChallenges();
                    gameState  = GameState.inGame;
                }
                else
                {
                    clearGesture(Gesture::start);
                }
            }
            break;
        case GameState::inGame:
            if (!challenges.empty())
            {
                auto& currentChallenge = *challenges.first();
                
                
                switch (challengeState)
                {
                    case ChallengeState::start:
                        outputUserMessage(currentChallenge.description());
                        challengeState = ChallengeState::inProgress;
                        challengeStartTime = std:chrono::steady_clock::now();
                        break;
                    case ChallengeState::inProgress:
                        if (isGestureDetected(currentChallenge.gesture()))
                        {
                            challengeCompleted(currentChallenge);
                            challenges.pop_front();
                            challengeState = ChallengeState::delayNext;
                        }
                        else
                        {const auto currentTime = std::chrono::steady_clock::now();
                            double challengeTimeout;
                            node.param("challenge_timeout", challengeTimeout, 10.0);
                            
                            if (currentTime >= challengeStartTime + challengeTimeout)
                            {
                                gameFailed();
                                
                            }
                        }
                        
                        
                        break;
                    case ChallengeState::delayNext:
                        // Add delay of X seconds
                        challengeState = ChallengeState::notStarted;
                        break;
                }
                
                if (isChallengeCompleted(*challenges.first()))
                {
                    challenges.pop_front();
                }
            }
            else
            {
                outputUserMessage(UserMessage::)
                gameState = GameState::NotStarted;
            }
            break;
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
