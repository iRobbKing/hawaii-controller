#ifndef HAWAII_ACCELERATOR_H
#define HAWAII_ACCELERATOR_H

#include <MPU6050_6Axis_MotionApps20.h>

namespace hawaii::workout::accelerator
{
    struct Vector
    {
        float x, y, z;
    };

    enum struct Error
    {
        None = 0,
        FailedToInitMpu = 1,
    };

    // TODO: dont expose ACCEL_FS
    // TODO: use this in calibration and calculating
    struct Config
    {
        ACCEL_FS sensitivity;
    };

    struct System
    {
        MPU6050 mpu;
        uint16_t mpu_dmp_current_packet_size;
    };

    auto init(System &accelerator, Config &config) -> void;
    auto reinit(System &accelerator) -> void;
    [[nodiscard]] auto get_acceleration(System &accelerator, Config &config, float &out_acceleration) -> bool;
}

#endif
