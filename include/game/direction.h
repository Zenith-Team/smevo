#pragma once

#include "types.h"

class Direction {
public:
    enum DirectionType {
        Right = 0,
        Left  = 1,
        Up    = 2,
        Down  = 3
    };

    static u32 directionToRotationList[];

    static DirectionType opposite(DirectionType direction) {
        switch (direction) {
            case Right:
                return Left;
            case Left: default:
                return Right;
            case Up:
                return Down;
            case Down:
                return Up;
        }
    }
};
