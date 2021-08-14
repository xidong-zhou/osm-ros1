/*
 * test.cpp
 *
 *  Created on: 17.10.2016
 *      Author: michal
 */

#include <ros/ros.h>
#include <osm_planner/CoorConv.hpp>
#include <std_msgs/Int32.h>
#include <std_srvs/Empty.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <osm_planner/newTarget.h>
#include <tf/tf.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/NavSatFix.h>


ros::Publisher position_pub;

double bais_x = 690975; 
double bais_y = 3119274;
int zone = 49;
double x_1 = 0.0;
double y_1 = 0.0;
geometry_msgs::Quaternion orientation;
osm_planner::newTarget gps_srv;
ros::ServiceClient set_source;

sensor_msgs::NavSatFix gps;

//getting path
void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg) 
{
    
    //ROS_INFO("x:%f,y:%f",msg->point.x,msg->point.y);
    double x = bais_x + msg->pose.position.x;
    double y = bais_y + msg->pose.position.y;
    double lat;
    double lon;
    WGS84Corr latlon;
    UTMXYToLatLon (x,y,zone,false,latlon);
    lat = RadToDeg(latlon.lat);
    lon = RadToDeg(latlon.log);
    //ROS_INFO("lat:%f,lon:%f",lat,lon);
    gps_srv.request.latitude = lat;
    gps_srv.request.longitude = lon;
    gps_srv.request.bearing = 0;
    set_source.call(gps_srv);
}


void initialposeCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg) 
{
    double x =  msg->pose.pose.position.x;
    double y =  msg->pose.pose.position.y;
    
    x_1 = bais_x + msg->pose.pose.position.x;
    y_1 = bais_y + msg->pose.pose.position.y;
    orientation = msg->pose.pose.orientation;
    tf::Quaternion quat;
    tf::quaternionMsgToTF(orientation, quat);  
    double roll, pitch, yaw;//定义存储r\p\y的容器
    tf::Matrix3x3(quat).getRPY(roll, pitch, yaw);//进行转换

    double lat;
    double lon;
    WGS84Corr latlon;
    UTMXYToLatLon (x_1,y_1,zone,false,latlon);
    lat = RadToDeg(latlon.lat);
    lon = RadToDeg(latlon.log);
    //ROS_INFO("lat:%f,lon:%f",lat,lon);
    gps.latitude = lat;
    gps.longitude = lon;
    gps.altitude = yaw;
    position_pub.publish(gps);


}


int main(int argc, char **argv) {

    ros::init(argc, argv, "convert");
    ros::NodeHandle n;
    
    set_source = n.serviceClient<osm_planner::newTarget>("make_plan");

    position_pub = n.advertise<sensor_msgs::NavSatFix>("gps", 10);
    ros::Subscriber goal_sub = n.subscribe("move_base_simple/goal", 1, &goalCallback);
    ros::Subscriber initialpose_sub = n.subscribe("initialpose", 1, &initialposeCallback);
    tf::TransformBroadcaster br;
    tf::Transform transform;
    tf::Quaternion q;    
    auto orientation_ = tf::createQuaternionMsgFromYaw(1.57);
    
    ros::Rate rate(20);
    
    while(ros::ok())
    {
	transform.setOrigin(tf::Vector3(x_1 - bais_x, y_1 - bais_y, 0.0) );
        tf::quaternionMsgToTF(orientation_, q);
	transform.setRotation(q);
	br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "/local_map", "/base_link"));
        ros::spinOnce();
    }


    return 0;
}
