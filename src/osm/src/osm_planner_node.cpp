/*
 * test.cpp
 *
 *  Created on: 17.10.2016
 *      Author: michal
 */

#include <osm_planner/osm_planner.h>
#include <sensor_msgs/NavSatFix.h>

class OsmPlannerNode: osm_planner::Planner{
public:

    OsmPlannerNode() : osm_planner::Planner(){

        //init ros topics and services
        ros::NodeHandle n;

        gps_topic_ = "gps";
        gps_sub = n.subscribe(gps_topic_, 1, &OsmPlannerNode::gpsCallback, this);

        //services
        //TODO mozno pridat namespace
        plan_service = n.advertiseService("make_plan", &OsmPlannerNode::makePlanCallback, this);
    }

    void update(){
      //  localization.updatePoseFromTF();
    }

private:

    /* Subscribers */
    ros::Subscriber gps_sub;
    ros::Subscriber odom_sub;

    /* Services */
    ros::ServiceServer plan_service;
    std::string gps_topic_;


    bool makePlanCallback(osm_planner::newTarget::Request &req, osm_planner::newTarget::Response &res) {

        //boost::shared_ptr<const nav_msgs::Odometry> odom = ros::topic::waitForMessage<nav_msgs::Odometry>(odom_topic_, ros::Duration(3));
        res.result = makePlan(req.latitude, req.longitude);
        return true;
    }

    void gpsCallback(const sensor_msgs::NavSatFix::ConstPtr& msg) {

        localization_source_->setPositionFromGPS(msg->latitude,msg->longitude,msg->altitude,true);
    }

};

int main(int argc, char **argv) {

	ros::init(argc, argv, "test_osm");

    OsmPlannerNode osm_planner;

    ros::Rate rate(2);

    while (ros::ok()) {

        osm_planner.update();
        ros::spinOnce();
        rate.sleep();
    }

return 0;
}
