/****************************************************************************************
 *
 * File:
 * 		ProximityVoterSuite.h
 *
 * Purpose:
 *
 * License:
 *      This file is subject to the terms and conditions defined in the file
 *      'LICENSE.txt', which is part of this source code package.
 *
 * Developer Notes:
 *
 ***************************************************************************************/

#pragma once

#include "../Navigation/LocalNavigationModule/Voters/ProximityVoter.h"
#include "../Tests/cxxtest/cxxtest/TestSuite.h"
#include "../WorldState/CollidableMgr/CollidableMgr.h"

class ProximityVoterSuite : public CxxTest::TestSuite {
   public:
    const int16_t maxVotes = 100;
    const int16_t weight = 1;

    void testAvoidOutsideVisualField() {
        CollidableMgr collidableManager;
        int bearingLowLimit = -10;
        int bearingHighLimit = 15;
        ProximityVoter proximityVoter(maxVotes, weight, collidableManager);
        proximityVoter.avoidOutsideVisualField(bearingLowLimit, bearingHighLimit);
        const ASRCourseBallot& ballot = proximityVoter.courseBallot;
        for (uint16_t i = 360 + bearingLowLimit; i < 360; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
        for (uint16_t i = 0; i < bearingHighLimit; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
        for (uint16_t i = bearingHighLimit; i < 360 + bearingLowLimit; ++i) {
            TS_ASSERT_LESS_THAN(ballot.get(i), 0);
        }
    }
    void testBearingAvoidanceSmoothed() {
        CollidableMgr collidableManager;
        ProximityVoter proximityVoter(maxVotes, weight, collidableManager);
        int testBearing = 37;
        const uint16_t avoidanceBearingRange = 10;  // copied from ProximityVoter
        auto relativeObstacleDistance =
            5;  // This must be a small number for the strict less than checks to pass
        proximityVoter.bearingAvoidanceSmoothed(testBearing, relativeObstacleDistance);
        const ASRCourseBallot& ballot = proximityVoter.courseBallot;
        for (uint16_t i = 0; i < testBearing - avoidanceBearingRange; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
        auto compareVote = 0;
        for (uint16_t i = testBearing - avoidanceBearingRange + 1; i < testBearing; ++i) {
            TS_ASSERT_LESS_THAN(ballot.get(i), compareVote);
            compareVote = ballot.get(i);
        }
        TS_ASSERT_DELTA(ballot.get(testBearing),
                        -1.5 * (100 - relativeObstacleDistance) * 2.0 / avoidanceBearingRange, 1.0);
        compareVote = ballot.get(testBearing);
        for (uint16_t i = testBearing + 1; i < testBearing + avoidanceBearingRange; ++i) {
            TS_ASSERT_LESS_THAN(compareVote, ballot.get(i));
            compareVote = ballot.get(i);
        }
        for (uint16_t i = testBearing + avoidanceBearingRange; i < 360; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
    }
    void testBearingPreferendeSmoothed() {
        CollidableMgr collidableManager;
        ProximityVoter proximityVoter(maxVotes, weight, collidableManager);
        int testBearing = 37;
        const uint16_t avoidanceBearingRange = 10;  // copied from ProximityVoter
        auto relativeObstacleDistance =
            2;  // This must be a small number for the strict less than checks to pass
        proximityVoter.bearingPreferenceSmoothed(testBearing, relativeObstacleDistance);
        const ASRCourseBallot& ballot = proximityVoter.courseBallot;
        for (uint16_t i = 0; i < testBearing + 90 - avoidanceBearingRange; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
        TS_ASSERT_LESS_THAN(0, ballot.get(testBearing + 90));
        auto compareVote = 0;
        for (uint16_t i = testBearing + 90 - avoidanceBearingRange + 1; i < testBearing + 90 + 1;
             ++i) {
            TS_ASSERT_LESS_THAN(compareVote, ballot.get(i));
            compareVote = ballot.get(i);
        }
        for (uint16_t i = testBearing + 90 + 1; i < testBearing + 90 + avoidanceBearingRange; ++i) {
            TS_ASSERT_LESS_THAN(ballot.get(i), compareVote);
            compareVote = ballot.get(i);
        }

        for (uint16_t i = testBearing + 90 + avoidanceBearingRange;
             i < testBearing + 270 - avoidanceBearingRange; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
        for (uint16_t i = 0; i < testBearing + 90 - avoidanceBearingRange; ++i) {
            TS_ASSERT_EQUALS(ballot.get(i), 0);
        }
    }
    void testVisualAvoidance() {
        CollidableMgr collidableManager;
        ProximityVoter proximityVoter(maxVotes, weight, collidableManager);
        std::map<int16_t, uint16_t> bearingToRelativeObstacleDistance;
        int visualRangeMin = -15;
        int visualRangeMax = 15;
        int obstacleRangeMin = -10;
        int obstacleRangeMax = 0;
        for (int i = visualRangeMin; i < obstacleRangeMin; ++i) {
            bearingToRelativeObstacleDistance[i] = 100;
        }
        for (int i = obstacleRangeMin; i < obstacleRangeMax; ++i) {
            bearingToRelativeObstacleDistance[i] = 25;
        }
        for (int i = obstacleRangeMax; i < visualRangeMax; ++i) {
            bearingToRelativeObstacleDistance[i] = 100;
        }
        collidableManager.addVisualField(bearingToRelativeObstacleDistance, 0);
        proximityVoter.visualAvoidance();
        const ASRCourseBallot& ballot = proximityVoter.courseBallot;
        int minBearing = 0;
        int maxBearing = 0;
        int minVote = ballot.maxVotes();
        int maxVote = -minVote;
        for (int i = 0; i < 360; ++i) {
            auto vote = ballot.get(i);
            if (vote > maxVote) {
                maxVote = vote;
                maxBearing = i;
            }
            if (vote < minVote) {
                minVote = vote;
                minBearing = i;
            }
        }
        TS_ASSERT_LESS_THAN(90 + obstacleRangeMin - 1, maxBearing);
        TS_ASSERT_LESS_THAN(maxBearing, 90 + obstacleRangeMax);
        TS_ASSERT_LESS_THAN(360 + obstacleRangeMin - 1, minBearing);
        TS_ASSERT_LESS_THAN(minBearing, 360 + obstacleRangeMax);
        TS_ASSERT_LESS_THAN(0, maxVote);
        TS_ASSERT_LESS_THAN(minVote, -ballot.maxVotes() * 0.9)

        obstacleRangeMin = -15;
        obstacleRangeMax = -10;
        for (int i = visualRangeMin; i < obstacleRangeMin; ++i) {
            bearingToRelativeObstacleDistance[i] = 100;
        }
        for (int i = obstacleRangeMin; i < obstacleRangeMax; ++i) {
            bearingToRelativeObstacleDistance[i] = 25;
        }
        for (int i = obstacleRangeMax; i < visualRangeMax; ++i) {
            bearingToRelativeObstacleDistance[i] = 100;
        }

        collidableManager.addVisualField(bearingToRelativeObstacleDistance, 0);
        proximityVoter.visualAvoidance();
        minBearing = 0;
        maxBearing = 0;
        minVote = ballot.maxVotes();
        maxVote = -minVote;
        for (int i = 0; i < 360; ++i) {
            auto vote = ballot.get(i);
            if (vote > maxVote) {
                maxVote = vote;
                maxBearing = i;
            }
            if (vote < minVote) {
                minVote = vote;
                minBearing = i;
            }
        }
        TS_ASSERT_LESS_THAN(maxBearing, 90 + obstacleRangeMax);
        TS_ASSERT_LESS_THAN(90 + obstacleRangeMin - 1, maxBearing);
        TS_ASSERT_LESS_THAN(minVote, -ballot.maxVotes() * 0.9)
    }
};