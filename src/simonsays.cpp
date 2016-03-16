#include "k2_client/BodyArray.h"
#include "ros/ros.h"
#include <vector>
#include <string>
#include "std_msgs/String.h"

namespace JointType
{
                         "SpineBase";break;
                          "SpineMid";break;
                         "Neck";break;
                          "Head";break;
                          "ShoulderLeft";break;
                         "ElbowLeft";break;
                      "WristLeft";break;
                     l"HandLeft";break;
                        "ShoulderRight";break;
                = "ElbowRight";break;
                  "WristRight";break;
                      "HandRight";break;
                     "HipLeft";break;
          "KneeLeft";break;
               "AnkleLeft";break;
                   "FootLeft";break;
                "HipRight";break;
                      "KneeRight";break;
                       "AnkleRight";break;
                        bool "FootRight";break;
                       bool "SpineShoulder";break;
                        bool"HandTipLeft";break;
                        bool "ThumbLeft";break;
                       bool "HandTipRight";break;
                        bool "ThumbRight";break;
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




int main(int argc, char **argv)
{
    
  /  ros::init(argc, argv, "listener");
  ros::NodeHandle n;

  ros::Subscriber sub = n.subscribe("head/kinect2/bodyArray", 1000, bodies_sub_cb);


    ros::spin();

  return 0;
}

