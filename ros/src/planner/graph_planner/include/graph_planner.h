/***********************************************************
 * 
 * @file: graph_planner.h
 * @breif: Contains the graph planner ROS wrapper class
 * @author: Yang Haodong
 * @update: 2022-10-26
 * @version: 1.0
 * 
 * Copyright (c) 2022， Yang Haodong
 * All rights reserved.
 * --------------------------------------------------------
 *
 **********************************************************/
#ifndef GRAPH_PLANNER_H
#define GRAPH_PLANNER_H

#include <nav_core/base_global_planner.h>
#include <nav_msgs/Path.h>
#include <nav_msgs/GetPlan.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Point.h>

#include "global_planner.h"

namespace graph_planner {
class GraphPlanner : public nav_core::BaseGlobalPlanner {
    public:
        /**
         * @brief  Constructor(default)
         */
        GraphPlanner();
        /**
         * @brief  Constructor
         * @param  name     planner name
         * @param  costmap  costmap pointer
         * @param  frame_id costmap frame ID
         */
        GraphPlanner(std::string name, costmap_2d::Costmap2D* costmap, std::string frame_id);
        /**
         * @brief Destructor
         * @return No return value
         * @details default
         */
        ~GraphPlanner();

        /**
         * @brief  Planner initialization
         * @param  name         planner name
         * @param  costmapRos   costmap ROS wrapper
         */
        void initialize(std::string name, costmap_2d::Costmap2DROS* costmapRos);
        /**
         * @brief  Planner initialization
         * @param  name     planner name
         * @param  costmap  costmap pointer
         * @param  frame_id costmap frame ID
         */
        void initialize(std::string name, costmap_2d::Costmap2D* costmap, std::string frame_id);
        /**
         * @brief plan a path given start and goal in world map
         * @param start     start in world map
         * @param goal      goal in world map
         * @param plan      plan
         * @param tolerance error tolerance
         * @return true if find a path successfully else false
         */
        bool makePlan(const geometry_msgs::PoseStamped& start, const geometry_msgs::PoseStamped& goal,
                    std::vector<geometry_msgs::PoseStamped>& plan);
        bool makePlan(const geometry_msgs::PoseStamped& start, const geometry_msgs::PoseStamped& goal, double tolerance,
                    std::vector<geometry_msgs::PoseStamped>& plan);
        /**
         * @brief  publish planning path
         * @param  path planning path
         */
        void publishPlan(const std::vector<geometry_msgs::PoseStamped>& plan);
        /**
         * @brief  regeister planning service
         * @param  req  request from client
         * @param  resp response from server
         */
        bool makePlanService(nav_msgs::GetPlan::Request& req, nav_msgs::GetPlan::Response& resp);
        /**
         * @brief  local costmap callback function
         * @param  costmap local costmap data
         */
        void localCostmapCallback(const nav_msgs::OccupancyGrid& local_costmap);


    protected:
        // costmap
        costmap_2d::Costmap2D* costmap_;
        // costmap frame ID
        std::string frame_id_;
        // path planning publisher
        ros::Publisher plan_pub_;
        // initialization flag
        bool initialized_;
        // global graph planner
        global_planner::GlobalPlanner* g_planner_;
        // nodes explorer publisher
        ros::Publisher expand_pub_;
        // planning service
        ros::ServiceServer make_plan_srv_;
        // planner name
        std::string planner_name_;
        // local costmap subscriber
        ros::Subscriber local_costmap_sub_;
        // local costmap pointer
        nav_msgs::OccupancyGrid* p_local_costmap_;

    private:
        // thread mutex
        boost::mutex mutex_;
        // offset of transform from world(x,y) to grid map(x,y)
        double convert_offset_;
        // tolerance
        double tolerance_;
        // whether outline the boudary of map
        bool is_outline_;
        // obstacle inflation factor
        double factor_;
        // whether publish expand map or not
        bool is_expand_;


    protected:
        /**
         * @brief  Inflate the boundary of costmap into obstacles to prevent cross planning
         * @param  costarr  costmap pointer
         * @param  nx       pixel number in costmap x direction
         * @param  ny       pixel number in costmap y direction
         */
        void _outlineMap(unsigned char* costarr, int nx, int ny);
        /**
         * @brief  publish expand zone
         * @param  expand  set of expand nodes
         */
        void _publishExpand(std::vector<Node> &expand);
        /**
         * @brief  calculate plan from planning path
         * @param  path path generated by global planner
         * @param  plan plan transfromed from path
         * @return bool true if successful else false
         */
        bool _getPlanFromPath(std::vector<Node> path, std::vector<geometry_msgs::PoseStamped>& plan);
        /**
         * @brief  tranform from costmap(x, y) to world map(x, y)
         * @param  mx costmap x
         * @param  my costmap y
         * @param  wx world map x
         * @param  wy world map y
         */
        void _mapToWorld(double mx, double my, double& wx, double& wy);
        /**
         * @brief  tranform from world map(x, y) to costmap(x, y)
         * @param  mx costmap x
         * @param  my costmap y
         * @param  wx world map x
         * @param  wy world map y
         */
        bool _worldToMap(double wx, double wy, double& mx, double& my);
};
}
#endif