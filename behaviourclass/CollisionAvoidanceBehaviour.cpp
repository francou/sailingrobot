//
// Created by Simon CHANU on 11/07/16.
//

#include "CollisionAvoidanceBehaviour.h"

//INPUT
/* Current_line (WP1 and WP2)
 * Boat_state
 *     GPS position
 *     roll, pitch, yaw
 *     speed
 * Sensors_output
 * sailing_zone (gps points)
 * wind direction and strength
 */
//IDEA
/* To use dynamic list of struct for obstacles and other variables such as this one
 */
//I WANT TO TRACK
/* boat position and heading
 * the obstacles with everything
 */
//SAVED CODE IN CASE OF ARCHITECTURAL CHANGE
/*

//sensorData update_sensor(){}

//    commandOutput run(INPUT){
//        sensors output = update_sensors(); //=> gives sensors output or compute an easier way to handle them
//        update_waypoints(); //=> update waypoints or compute an easier way to handle them
//        check_obstacles();
//    //    update_map();
//        if(these_obstacles_are_a_problem()){
//            Eigen::MatrixXd potential_field = compute_potential_field();
//            minPotField min = find_minimum_potential_field(potential_field);
//            compute_new_path();
//        }
//        return compute_commands();
//    }


*/

//CONSTRUCTOR

/*
 * Super class call. Calls the database
 */
CollisionAvoidanceBehaviour::CollisionAvoidanceBehaviour(DBHandler *db) :
        RoutingBehaviour(db) {

}

//UTILITY FUNCTIONS

/*
 * Might be put into Utility class soon. TODO : Utility class ?
 * The angles must be in radians. It's radAngle1-radAngle2.
 */
double CollisionAvoidanceBehaviour::angleDiff(
        double radAngle1,
        double radAngle2) {
    return fmod(radAngle1-radAngle2+M_PI,2*M_PI)+M_PI;
}

/*
 * Haversine algorithm for distance computation on Earth.
 * Took on http://www.movable-type.co.uk/scripts/latlong.html
 * a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
 * c = 2 ⋅ atan2( √a, √(1−a) )
 * distance = Rearth ⋅ c
 *
 * Maybe this function should as well be in the Utility class. TODO : Utility class ?
 */
double CollisionAvoidanceBehaviour::calculateDistance(
        Eigen::Vector2d point1,
        Eigen::Vector2d point2) {
    //...(1) : latitude //...(2) : longitude
    const double deltaLat = angleDiff(point2(1),point1(1));
    const double deltaLon = angleDiff(point2(2),point1(2));
    const double Rearth = 6371000;
    const double a  = sin(deltaLat/2)*sin(deltaLat/2)
                      + cos(point1(1))*cos(point2(1))*sin(deltaLon/2)*sin(deltaLon/2);
    const double c = 2*atan2(sqrt(a),sqrt(1-a));
    return Rearth*c;
}

/*
 * Debugging functions, specific to Eigen
 */
void CollisionAvoidanceBehaviour::printStdVectorMat(
        std::string const &name,
        std::vector<Eigen::MatrixXd> const &v) {

    std::cout << " " << std::endl;
    std::cout << name << " : " << std::endl;
    std::cout << " " << std::endl;
    for (int i = 0; i < (int) v.size(); ++i) {
        std::cout << v[i] << std::endl;
        std::cout << " " << std::endl;
    }
}

void CollisionAvoidanceBehaviour::printStdVectorFloat(
        std::string const &name,
        std::vector<float> const &v) {

    std::cout << " " << std::endl;
    std::cout << name << " : " << std::endl;
    std::cout << " " << std::endl;
    for (int i = 0; i < (int) v.size(); ++i) {
        std::cout << v[i] << std::endl;
        std::cout << " " << std::endl;
    }
}

void CollisionAvoidanceBehaviour::printMat(
        std::string const &name,
        Eigen::MatrixXd const &mat) {

    std::cout << " " << std::endl;
    std::cout << name << " : " << std::endl;
    std::cout << " " << std::endl;
    std::cout << mat << std::endl;
    std::cout << " " << std::endl;
}

//PRIVATE MAIN FUNCTIONS

/*
 * Makes the interface between the old code and the new one. This is make the code more modular
 * since changes in the architecture might come.
 */
SensorData CollisionAvoidanceBehaviour::update_sensors(
        SystemStateModel &systemStateModel,
        const Simulation sim) {
    SensorData sensorData;
    if (sim.waypoints) {
        // TODO : simulation part (called or made here)
    }
    else {
        //Extraction of data from sensors

        //Position and speed
        //The latitude and longitude are easier to compute in radians
        sensorData.gpsLat = Utility::degreeToRadian(systemStateModel.gpsModel.positionModel.latitude);
        sensorData.gpsLon = Utility::degreeToRadian(systemStateModel.gpsModel.positionModel.longitude);
        sensorData.gpsSpeed = systemStateModel.gpsModel.speed;

        //Heading
        //compHeading : degree from north -> radian from east
        sensorData.compHeading = Utility::degreeToRadian(systemStateModel.compassModel.heading)
                                 - M_PI / 2;
        //gpsHeading : degree from north -> radian from east
        sensorData.gpsHeading = Utility::degreeToRadian(systemStateModel.gpsModel.heading)
                                - M_PI / 2;

        //Wind
        //windDirection : degree from north -> radian from east
        sensorData.windDirection = Utility::degreeToRadian(systemStateModel.windsensorModel.direction)
                                   - M_PI / 2;
        sensorData.windSpeed = systemStateModel.windsensorModel.speed;

        //Tilt
        sensorData.pitch = systemStateModel.compassModel.pitch;
        sensorData.roll = systemStateModel.compassModel.roll;
    }

    //ASSUMPTION
    /*
     * The sensors will give a confidence interval of the heading and the distance
     * relatively to the boat
     */
    if (sim.obstacles) {
        //Mock obstacles here
        // [UPDATE] Elouan said this script will have to take care of the obstacles without his simulator.
        // TODO : Create a simulation class/node which will compute every sensor output. I might only have to modify Elouan's

        // This don't work right now, this is an example.
        // TODO : make working mock obstacles
        ObstacleData obstacle0 = {2,  //double minDistanceToObstacle;
                                  20, //double maxDistanceToObstacle;
                                  -10,//double LeftBoundheadingRelativeToBoat;
                                  10};//double RightBoundheadingRelativeToBoat;
        sensorData.detectedObstacles.push_back(obstacle0);
        ObstacleData obstacle1 = {2,  //double minDistanceToObstacle;
                                  20, //double maxDistanceToObstacle;
                                  -10,//double LeftBoundheadingRelativeToBoat;
                                  10};//double RightBoundheadingRelativeToBoat;
        sensorData.detectedObstacles.push_back(obstacle1);
        ObstacleData obstacle2 = {2,  //double minDistanceToObstacle;
                                  20, //double maxDistanceToObstacle;
                                  -10,//double LeftBoundheadingRelativeToBoat;
                                  10};//double RightBoundheadingRelativeToBoat;
        sensorData.detectedObstacles.push_back(obstacle2);
        ObstacleData obstacle3 = {2,  //double minDistanceToObstacle;
                                  20, //double maxDistanceToObstacle;
                                  -10,//double LeftBoundheadingRelativeToBoat;
                                  10};//double RightBoundheadingRelativeToBoat;
        sensorData.detectedObstacles.push_back(obstacle3);

    }
    else {
        // TODO : when the sensors wil be ready, put the code to get everything here

    }

    return sensorData;
}

/*
 * Update waypoints or compute an easier way to handle them
 */
//    void update_waypoints(){}

/*
 * Is there any obstacles ? If yes, which information can i gather on them.
 * OUTPUT a list of obstacles (struct) with all their characteristics.
 *
 * For now transform hardcoded obstacles to something more usable.
 */
std::vector<Obstacle> CollisionAvoidanceBehaviour::check_obstacles(SensorData sensorData) {
    //Clean obstacles
    int i = 0;
    while(i<=seenObstacles.size()){
        //the current memorized obstacle does not belong to detectedObstacles
        /*
         * The computations made here verify that an obstacle isn't too far from any other detected
         * obstacle and is not detected : that would mean that it doesn't exist any more and then need
         * to be cleaned from the memory
         */
        const double headingCenterOfMemorizedObstacle =
                angleDiff(seenObstacles[i].leftBoundHeading,seenObstacles[i].rightBoundHeading);
        const double widthOfMemorizedObstacleAtClosest =
                seenObstacles[i].lowerBoundDistance
                * tan(angleDiff(seenObstacles[i].leftBoundHeading,seenObstacles[i].rightBoundHeading)/2);

        const Eigen::Vector2d closestCenterOfMemorizedObstacleAtDetection(
                seenObstacles[i].xGPSBoatPositionAtDetection
                + seenObstacles[i].lowerBoundDistance*cos(headingCenterOfMemorizedObstacle),// x
                seenObstacles[i].yGPSBoatPositionAtDetection
                + seenObstacles[i].lowerBoundDistance*sin(headingCenterOfMemorizedObstacle) // y
        );

        for(int j = 0;j<sensorData.detectedObstacles.size();j++) {
            const double headingCenterOfDetectedObstacle =
                    angleDiff(sensorData.detectedObstacles[j].LeftBoundHeadingRelativeToBoat,
                              sensorData.detectedObstacles[j].RightBoundHeadingRelativeToBoat)
                    + sensorData.compHeading;
            const double widthOfCurrentDetectedObstacleAtClosest =
                    sensorData.detectedObstacles[j].minDistanceToObstacle
                    * tan(angleDiff(sensorData.detectedObstacles[j].LeftBoundHeadingRelativeToBoat,
                                    sensorData.detectedObstacles[j].RightBoundHeadingRelativeToBoat)
                          /2);

            const Eigen::Vector2d closestCenterOfCurrentlyDetectedObstacle(
                    sensorData.gpsLat
                    + sensorData.detectedObstacles[j].minDistanceToObstacle
                      * cos(headingCenterOfDetectedObstacle),// x
                    sensorData.gpsLon
                    + sensorData.detectedObstacles[j].minDistanceToObstacle
                      * sin(headingCenterOfDetectedObstacle) // y
            );

            const Eigen::Vector2d vectorBetweenClosestPointFromObstacles =
                    closestCenterOfMemorizedObstacleAtDetection
                    - closestCenterOfCurrentlyDetectedObstacle;
            // the IDE says there is an error here but it seems correct

            //Conditions
            const bool theObstaclesAreNotTooClose = vectorBetweenClosestPointFromObstacles.norm()
                                                    + widthOfCurrentDetectedObstacleAtClosest
                                                    + widthOfMemorizedObstacleAtClosest
                                                    > distNotTheSameObstacle;
            const bool theObstacleShouldHaveBeenDetected =


            if(){
                // Remove the undetected obstacle from the memory
            }
        }
        i++;
    }

    //Register new obstacles
    for(i = 0;i<sensorData.detectedObstacles.size();i++){
        Obstacle newObstacle;
        newObstacle.xGPSBoatPositionAtDetection = sensorData.gpsLat;
        newObstacle.yGPSBoatPositionAtDetection = sensorData.gpsLon;

        newObstacle.leftBoundHeading = sensorData.detectedObstacles[i].LeftBoundHeadingRelativeToBoat
                                       + sensorData.compHeading;
        newObstacle.rightBoundHeading = sensorData.detectedObstacles[i].RightBoundHeadingRelativeToBoat
                                        + sensorData.compHeading;
        newObstacle.lowerBoundDistance = sensorData.detectedObstacles[i].minDistanceToObstacle;
        newObstacle.upperBoundDistance = sensorData.detectedObstacles[i].maxDistanceToObstacle;
        newObstacle.color = "Null";

        // Add new obstacle to the list
        seenObstacles.push_back(newObstacle);
    }
}

/*
 *
 */
//map update_map(){}

/*
 * Check if there is intersection between the current path+security radius and the obstacle
 */
bool CollisionAvoidanceBehaviour::these_obstacles_are_a_problem(
        std::vector<Obstacle> seenObstacles) { // OUTPUT if these obstacles are a problem
    bool theseObstaclesAreAProblem;


    return theseObstaclesAreAProblem;
}

/*
 * Compute the potential field for the obstacles, the sailing zone,
 * the boat, the objective, and the wind.
 *
 * For now it creates the matrix at each loop (easier for code review)
 * The size of the matrix needs to be adapted to the size of the sailing zone
 * (with the max and mins for example)
 */
Eigen::MatrixXd CollisionAvoidanceBehaviour::compute_potential_field(
        std::vector<Obstacle> seen_obstacles,
        std::vector<Eigen::Vector2d> sailing_zone,
        FollowedLine) {

}

/*
 * Find the minimum in the potential field and return its coordinates in the matrix
 * as well as its real gps coordinates.
 */
MinPotField CollisionAvoidanceBehaviour::find_minimum_potential_field(
        Eigen::MatrixXd Potential_field) {

}

/*
 * Gives the new line to follow. It would be better if it added a WP in the DataBase as well.
 */
FollowedLine CollisionAvoidanceBehaviour::compute_new_path(
        MinPotField min) {

}

/*
 * Compute the commands according to the new path.
 * Follow the line between the last waypoint and the next
 */
CommandOutput CollisionAvoidanceBehaviour::compute_commands(
        FollowedLine line) {

}

/*
 * The most important function of the class, it calls al the others.
 *
 * Some inputs of the function are class variables. This is to improve
 * the readability of the code and its modularity in case of architectural
 * modifications.
 *
 * I don't think inputing system state into this function is the best idea.
 * But update_sensors need it. Either I move update_sensors out of run(), in
 * that case isn't any more a modular function since it lacks update_sensors,
 * or systemStateModel stays where it is for the same result.
 *
 * TODO : When the message architecture will be done, modify all this.
 */
CommandOutput CollisionAvoidanceBehaviour::run(
        SystemStateModel &systemStateModel) {
    //Note on simulation
    /*
     * For now i don't know of any place in the code where it specified if the code is
     * in a simulated environement or not. So this variable is temporary.
     */
    Simulation sim = {false, // waypoints
                      true};// obstacles

    //Gives sensors output or compute an easier way to handle them
    sensorOutput = update_sensors(systemStateModel,
                                  sim);

    //update_waypoints(); //Update waypoints or compute an easier way to handle them

    seenObstacles = check_obstacles(sensorOutput);
    //    update_map();
    if (these_obstacles_are_a_problem(seenObstacles)) {
        Eigen::MatrixXd potential_field = compute_potential_field(seenObstacles,
                                                                  sailingZone,
                                                                  followedLine);
        MinPotField min = find_minimum_potential_field(potential_field);
        followedLine = compute_new_path(min);
    }
    return compute_commands(followedLine);
}

//PUBLIC FUNCTIONS

/*
 * Initialize values :
 *  sailingZone
 *
 *  TODO : replace hardcoded sailing zone by an import from the database
 */
bool CollisionAvoidanceBehaviour::init() {

    //SailingZone initialization clockwise (x,y)
    Eigen::Vector2d GPSpoint0(-1, 1);
    Eigen::Vector2d GPSpoint1(1, 1);
    Eigen::Vector2d GPSpoint2(1, -1);
    Eigen::Vector2d GPSpoint3(-1, -1);
    sailingZone = {GPSpoint0,
                   GPSpoint1,
                   GPSpoint2,
                   GPSpoint3};

}

/*
 * This is a trick to interface this code and the rest of the c++ code
 */
void CollisionAvoidanceBehaviour::computeCommands(
        SystemStateModel &systemStateModel,
        std::unique_ptr<Position> const &position,
        double trueWindDirection,
        bool mockPosition,
        bool getHeadingFromCompass) {
    CommandOutput out = run(systemStateModel);
    m_rudderCommand = out.deltaRudder;
    m_sailCommand = out.deltaSail;
}

