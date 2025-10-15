#include "accelerator.hpp"
#include <Arduino.h>

namespace
{
    namespace hwa = hawaii::workout::accelerator;

    // Считывание показателей акселерометра.
    void mean_sensors(
        MPU6050 mpu,
        int *iMean // Массив вычисляемых предполагаемых отклонений.
    )
    {
        long buff_ax = 0, buff_ay = 0, buff_az = 0, buff_gx = 0, buff_gy = 0, buff_gz = 0;
        int16_t ax, ay, az, gx, gy, gz;  // Текущие показания акселерометра.
        const int bufferSize = 70;       // Количество итераций измерений среднего значения.
        const int skipMeasurements = 10; // Количество неучитываемых итераций калибровки гироскопа.
                                         // Читаем данные гироскопа.
        for (int i = 0; i < (bufferSize + skipMeasurements); i++)
        {
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
            // Первые skipMeasurements измерений пропускаем.
            if (i >= skipMeasurements && i <= (bufferSize + skipMeasurements))
            {
                buff_ax += ax;
                buff_ay += ay;
                buff_az += az;
                buff_gx += gx;
                buff_gy += gy;
                buff_gz += gz;
            }
            // В последней итерации вычисляем усредненные показатели.
            if (i == (bufferSize + skipMeasurements - 1))
            {
                iMean[0] = buff_ax / bufferSize;
                iMean[1] = buff_ay / bufferSize;
                iMean[2] = buff_az / bufferSize;
                iMean[3] = buff_gx / bufferSize;
                iMean[4] = buff_gy / bufferSize;
                iMean[5] = buff_gz / bufferSize;
            }
            // Дискретность изменрений 2 миллисекунды.
            delay(2);
        }
    }

    int ax_offset, ay_offset, az_offset; // Вычисляемы значения отклонений.
    int gx_offset, gy_offset, gz_offset;
    int mean[6] = {0, 0, 0, 0, 0, 0}; // Предполагаемое отклонение.

    void calibrate(MPU6050 mpu)
    {
        int constexpr acel_deadzone = 10;     // точность калибровки акселерометра (по умолчанию 8).
        int constexpr gyro_deadzone = 6;      // точность калибровки гироскопа (по умолчанию 2).

        byte constexpr max_calibrations = 30;
        // Задаем диапазон ускорений в +/-16G. Знаменатель 2048.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
        // Задаем диапазон ускорений в +/-8G.  Знаменатель 4096.
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
        // Задаем диапазон ускорений в +/-4G.  Знаменатель 8192.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        // Задаем диапазон ускорений в +/-2G.  Знаменатель 16384.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        // Задаем максимальную угловуюскорость +/-250м/с
        // mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
        // Сброс показателей акселерометра.
        mpu.setXAccelOffset(0);
        mpu.setYAccelOffset(0);
        mpu.setZAccelOffset(0);
        mpu.setXGyroOffset(0);
        mpu.setYGyroOffset(0);
        mpu.setZGyroOffset(0);

        ax_offset = -mean[0] / 8;
        ay_offset = -mean[1] / 8;
        az_offset = -mean[2] / 8;
        gx_offset = -mean[3] / 4;
        gy_offset = -mean[4] / 4;
        gz_offset = -mean[5] / 4;

        for (byte i = 0; i < max_calibrations; ++i)
        {
            int ready = 0;

            mpu.setXAccelOffset(ax_offset);
            mpu.setYAccelOffset(ay_offset);
            mpu.setZAccelOffset(az_offset);
            mpu.setXGyroOffset(gx_offset);
            mpu.setYGyroOffset(gy_offset);
            mpu.setZGyroOffset(gz_offset);

            mean_sensors(mpu, mean);

            if (abs(mean[0]) <= acel_deadzone)
                ++ready;
            else
                ax_offset -= mean[0] / acel_deadzone;

            if (abs(mean[1]) <= acel_deadzone)
                ++ready;
            else
                ay_offset -= mean[1] / acel_deadzone;

            if (abs(mean[2]) <= acel_deadzone)
                ++ready;
            else
                az_offset -= mean[2] / acel_deadzone;

            if (abs(mean[3]) <= gyro_deadzone)
                ++ready;
            else
                gx_offset -= mean[3] / (mean[3] + 1);

            if (abs(mean[4]) <= gyro_deadzone)
                ++ready;
            else
                gy_offset -= mean[4] / (gyro_deadzone + 1);

            if (abs(mean[5]) <= gyro_deadzone)
                ++ready;
            else
                gz_offset -= mean[5] / (gyro_deadzone + 1);
            // Выходим, если процесс калибровки будет выполне
            // менее чем заданное число итераций.
            if (ready == 6)
                break;
        }
    }
}

namespace hawaii::workout::accelerator
{
    auto init(System &accelerator, Config &config) -> void
    {
        accelerator.mpu.initialize();

        calibrate(accelerator.mpu);
    }

    auto reinit(System &accelerator) -> void
    {
        accelerator.mpu.initialize();

        accelerator.mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);

        accelerator.mpu.setXAccelOffset(ax_offset);
        accelerator.mpu.setYAccelOffset(ay_offset);
        accelerator.mpu.setZAccelOffset(az_offset);
        accelerator.mpu.setXGyroOffset(gx_offset);
        accelerator.mpu.setYGyroOffset(gy_offset);
        accelerator.mpu.setZGyroOffset(gz_offset);
    }

    auto get_acceleration(System &accelerator, Config &config, float &out_acceleration) -> bool
    {
        int constexpr samples = 60;
        double average_acceleration = 0;

        for (int i = 0; i < samples; ++i)
        {
            if (Wire.getWireTimeoutFlag())
                return false;

            int16_t ax, ay, az;
            accelerator.mpu.getAcceleration(&ax, &ay, &az);

            float constexpr g = 4096.0f;

            double const a_mc2_x = (double)ax / g;
            double const a_mc2_y = (double)ay / g;

            double const acceleration = sqrt(a_mc2_x * a_mc2_x + a_mc2_y * a_mc2_y);

            average_acceleration += acceleration / samples;
        }

        out_acceleration = average_acceleration;
        return true;
    }
}