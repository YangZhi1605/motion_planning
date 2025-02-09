/***********************************************************
 * 
 * @file: jump_point_search.cpp
 * @breif: Contains the Jump Point Search(JPS) planner class
 * @author: Yang Haodong
 * @update: 2022-10-27
 * @version: 1.0
 * 
 * Copyright (c) 2022， Yang Haodong
 * All rights reserved.
 * --------------------------------------------------------
 *
 **********************************************************/
#include "jump_point_search.h"

namespace jps_planner {
    /**
     * @brief  Constructor
     * @param   nx          pixel number in costmap x direction
     * @param   ny          pixel number in costmap y direction
     * @param   resolution  costmap resolution
     */
    JumpPointSearch::JumpPointSearch(int nx, int ny, double resolution) : GlobalPlanner(nx, ny, resolution){}

    /**
     * @brief Jump Point Search(JPS) implementation
     * @param costs     costmap
     * @param start     start node
     * @param goal      goal node
     * @param expand    containing the node been search during the process
     * @return tuple contatining a bool as to whether a path was found, and the path
     */
    std::tuple<bool, std::vector<Node>> JumpPointSearch::plan(const unsigned char* costs, const Node& start,
                                                const Node& goal, std::vector<Node> &expand) {
        // copy
        this->costs_ = costs;
        this->start_ = start, this->goal_ = goal;

        // open list
        std::priority_queue<Node, std::vector<Node>, compare_cost> open_list;
        open_list.push(start);

        // closed list
        std::unordered_set<Node, NodeIdAsHash, compare_coordinates> closed_list;

        // expand list
        expand.clear();
        expand.push_back(start);

        // get all possible motions
        std::vector<Node> motions = getMotion();

        // main loop
        while (!open_list.empty()) {
            // pop current node from open list
            Node current = open_list.top();
            open_list.pop();

            // current node do not exist in closed list
            if (closed_list.find(current) != closed_list.end())
                continue;

            // goal found
            if (current == goal) {
                closed_list.insert(current);
                return {true, this->_convertClosedListToPath(closed_list, start, goal)};
            }

            // explore neighbor of current node
            std::vector<Node> jp_list;
            for (const auto& motion : motions) {
                Node jp = this->jump(current, motion);

                // exists and not in CLOSED set
                if (jp.id != -1 && closed_list.find(jp) == closed_list.end()) {
                    jp.pid = current.id;
                    jp.h_cost = std::sqrt(std::pow(jp.x - goal.x, 2)
                                + std::pow(jp.y - goal.y, 2));
                    jp_list.push_back(jp);
                }
            }

            for (const auto& jp : jp_list) {
                open_list.push(jp);
                expand.push_back(jp);

                // goal found
                if (jp == goal)
                    break;
            }
            
            closed_list.insert(current);
        }
        return {false, {}};
    }
           
    /**
     * @brief detect whether current node has forced neighbor or not 
     * @param point     current node
     * @param motion    the motion that current node executes
     * @return true if current node has forced neighbor else false
     */
    bool JumpPointSearch::detectForceNeighbor(const Node& point, const Node& motion) {
        int x = point.x;
        int y = point.y;
        int x_dir = motion.x;
        int y_dir = motion.y;

        // horizontal
        if (x_dir && !y_dir) {
            if (this->costs_[this->grid2Index(x, y + 1)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x + x_dir, y + 1)] < this->lethal_cost_ * this->factor_)
                return true;
            if (this->costs_[this->grid2Index(x, y - 1)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x + x_dir, y - 1)] < this->lethal_cost_ * this->factor_)
                return true;
        }
        
        // vertical
        if (!x_dir && y_dir) {
            if (this->costs_[this->grid2Index(x + 1, y)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x + 1, y + y_dir)] < this->lethal_cost_ * this->factor_)
                return true;
            if (this->costs_[this->grid2Index(x - 1, y)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x - 1, y + y_dir)] < this->lethal_cost_ * this->factor_)
                return true;
        }

        // diagonal
        if (x_dir && y_dir) {
            if (this->costs_[this->grid2Index(x - x_dir, y)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x - x_dir, y + y_dir)] < this->lethal_cost_ * this->factor_)
                return true;
            if (this->costs_[this->grid2Index(x, y - y_dir)] >= this->lethal_cost_ * this->factor_ &&
                this->costs_[this->grid2Index(x + x_dir, y - y_dir)] < this->lethal_cost_ * this->factor_)
                return true;
        }

        return false;
    }
    
    /**
     * @brief calculate jump node recursively
     * @param point     current node
     * @param motion    the motion that current node executes
     * @return jump node
     */   
    Node JumpPointSearch::jump(const Node& point, const Node& motion) {
        Node new_point = point + motion;
        new_point.id = this->grid2Index(new_point.x, new_point.y);
        new_point.pid = point.id;
        new_point.h_cost = std::sqrt(std::pow(new_point.x - this->goal_.x, 2)
                            + std::pow(new_point.y - this->goal_.y, 2));

        // next node hit the boundary or obstacle
        if (new_point.id < 0 || new_point.id >= this->ns_ || 
            this->costs_[new_point.id] >= this->lethal_cost_ * this->factor_)
            return Node(-1, -1, -1, -1, -1, -1);
        
        // goal found
        if (new_point == this->goal_)
            return new_point;

        // diagonal
        if (motion.x && motion.y) {
            // if exists jump point at horizontal or vertical
            Node x_dir = Node(motion.x, 0, 1, 0, 0, 0);
            Node y_dir = Node(0, motion.y, 1, 0, 0, 0);
            if (this->jump(new_point, x_dir).id != -1 || 
                this->jump(new_point, y_dir).id != -1)
                return new_point;
        }

        // exists forced neighbor
        if (this->detectForceNeighbor(new_point, motion))
            return new_point;
        else
            return this->jump(new_point, motion);
    }
}