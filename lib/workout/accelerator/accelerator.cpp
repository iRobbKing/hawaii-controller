#include "accelerator.hpp"

namespace
{
    namespace hwa = hawaii::workout::accelerator;

    [[nodiscard]] auto init_mpu(hwa::System &accelerator) -> bool
    {
        accelerator.mpu.initialize();

        return true;
    }

    // [[nodiscard]] auto get_accelerator_sensitivity_range(ACCEL_FS sensitivity) -> uint8_t
    // {
    //     switch (sensitivity)
    //     {
    //     case ACCEL_FS::A2G:
    //         return MPU6050_ACCEL_FS_2;
    //     case ACCEL_FS::A4G:
    //         return MPU6050_ACCEL_FS_4;
    //     case ACCEL_FS::A8G:
    //         return MPU6050_ACCEL_FS_8;
    //     case ACCEL_FS::A16G:
    //         return MPU6050_ACCEL_FS_16;
    //     }
    // }

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
        const int acel_deadzone = 10;     // точность калибровки акселерометра (по умолчанию 8).
        const int gyro_deadzone = 6;      // точность калибровки гироскопа (по умолчанию 2).

        // Максимальное количество итераций для калибровки.
        byte vNumColibrations = 30;
        // Задаем диапазон ускорений в +/-16G. Знаменатель 2048.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
        // Задаем диапазон ускорений в +/-8G.  Знаменатель 4096.
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
        // Задаем диапазон ускорений в +/-4G.  Знаменатель 8192.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        // Задаем диапазон ускорений в +/-2G.  Знаменатель 16384.
        // mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        // Задаем максимальную угловуюскорость +/-250м/с
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
        // Сброс показателей акселерометра.
        mpu.setXAccelOffset(0);
        mpu.setYAccelOffset(0);
        mpu.setZAccelOffset(0);
        mpu.setXGyroOffset(0);
        mpu.setYGyroOffset(0);
        mpu.setZGyroOffset(0);

        ax_offset = -mean[0] / 8; // 8 - размер байта в микроконтроллере??? Обсудить.
        ay_offset = -mean[1] / 8;
        az_offset = -mean[2] / 8;
        gx_offset = -mean[3] / 4; // 4 - размер байта в микроконтроллере??? Обсудить.
        gy_offset = -mean[4] / 4;
        gz_offset = -mean[5] / 4;

        // Калибровка очень точный процесс и может идти долго.
        // Поэтому делаем заданное число измерений.
        // Но, если кабировка пройдет быстрее, то процесс прервется.
        for (byte vCnt = 0; vCnt <= vNumColibrations; vCnt++)
        {
            int ready = 0;
            mpu.setXAccelOffset(ax_offset);
            mpu.setYAccelOffset(ay_offset);
            mpu.setZAccelOffset(az_offset);
            mpu.setXGyroOffset(gx_offset);
            mpu.setYGyroOffset(gy_offset);
            mpu.setZGyroOffset(gz_offset);

            // Вычисление средних показателей.
            mean_sensors(mpu, mean);
            if (abs(mean[0]) <= acel_deadzone)
                ready++;
            else
                ax_offset -= mean[0] / acel_deadzone;

            if (abs(mean[1]) <= acel_deadzone)
                ready++;
            else
                ay_offset -= mean[1] / acel_deadzone;

            if (abs(mean[2]) <= acel_deadzone)
                ready++;
            else
                az_offset -= mean[2] / acel_deadzone;

            if (abs(mean[3]) <= gyro_deadzone)
                ready++;
            else
                gx_offset -= mean[3] / (mean[3] + 1);

            if (abs(mean[4]) <= gyro_deadzone)
                ready++;
            else
                gy_offset -= mean[4] / (gyro_deadzone + 1);

            if (abs(mean[5]) <= gyro_deadzone)
                ready++;
            else
                gz_offset -= mean[5] / (gyro_deadzone + 1);
            // Выходим, если процесс калибровки будет выполне
            // менее чем заданное число итераций.
            if (ready == 6)
                break;
        }
    }

    auto calibrate_mpu(hwa::System &accelerator, hwa::Config &config) -> void
    {
        calibrate(accelerator.mpu);
    }

    // auto set_mpu_interrupt(hwa::System &accelerator, hwa::Config &config) -> void
    // {
    //     accelerator.mpu.setSleepEnabled(false);

    //     accelerator.mpu.setMotionDetectionThreshold(config.motion_threshold_lsb);
    //     accelerator.mpu.setMotionDetectionDuration(config.motion_duration_threshold_ms);
    //     accelerator.mpu.setIntMotionEnabled(true);

    //     accelerator.mpu.setIntFIFOBufferOverflowEnabled(true);

    //     accelerator.mpu.setIntDMPEnabled(true);
    //     accelerator.mpu.setInterruptLatch(true);
    //     accelerator.mpu.setInterruptLatchClear(true);
    // }

    // auto init_mpu_dmp(hwa::System &accelerator) -> bool
    // {
    //     if (accelerator.mpu.dmpInitialize() != 0)
    //         return false;

    //     accelerator.mpu.setDMPEnabled(true);

    //     return true;
    // }
}

namespace hawaii::workout::accelerator
{
    auto init(System &accelerator, Config &config) -> Error
    {
        if (!init_mpu(accelerator))
            return Error::FailedToInitMpu;
        // if (!init_mpu_dmp(accelerator))
        //     return Error::FailedToInitDmp;
        calibrate_mpu(accelerator, config);
        // Serial.println("calibrated");
        // set_mpu_interrupt(accelerator, config);
        // Serial.println("interrupt set");

        return Error::None;
    }

    auto is_connected(System &accelerator) -> bool
    {
        return accelerator.mpu.testConnection();
    }

    // bool is_disconnected = false;

    auto get_acceleration(System &accelerator, Config &config) -> float
    {
        double average_acceleration = 0;

        // if (is_disconnected)
        // {
        //     if (accelerator.mpu.initialize())
        // }

        // if (!is_connected(accelerator))
        // {
        //     is_disconnected = true;
        //     return 0.0f;
        // }

        int constexpr sample = 60;
        for (int i = 0; i < sample; ++i)
        {
            // Все пробивают 2G и 4G, пожтому перезодим на 8G.
            // 16g = 2048, 8g = 4096, 4g = 8192, 2g = 16384;
            // float constexpr gValue = 16384.0f;
            float constexpr gValue = 4096.0f;

            // читаем показания
            int16_t ax, ay, az;
            accelerator.mpu.getAcceleration(&ax, &ay, &az);

            // переводим в координаты вектора, только в плоскости
            double const aMc2X = (double)ax / gValue;
            double const aMc2Y = (double)ay / gValue;

            // возращаем длину вектора
            double const acceleration = sqrt(aMc2X * aMc2X + aMc2Y * aMc2Y);

            average_acceleration += acceleration / sample;
        }

        return average_acceleration;
    }

    // auto _get_acceleration(System &accelerator, Config &config, Vector &out_acceleration) -> Error
    // {
    //     uint16_t const mpu_dmp_packet_size = accelerator.mpu.dmpGetFIFOPacketSize();
    //     uint8_t const mpu_int_status = accelerator.mpu.getIntStatus();
    //     accelerator.mpu_dmp_current_packet_size = accelerator.mpu.getFIFOCount();

    //     if ((mpu_int_status & (1 << MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || accelerator.mpu_dmp_current_packet_size >= 1024)
    //     {
    //         // if overflowed
    //         accelerator.mpu.resetFIFO();
    //         accelerator.mpu_dmp_current_packet_size = accelerator.mpu.getFIFOCount();
    //         // TODO: this shouldn be error?? handle restart mid training?
    //         // return Error::MpuFifoOverflow;
    //     }
    //     else if (mpu_int_status & (1 << MPU6050_INTERRUPT_DMP_INT_BIT))
    //     {
    //         // if interruped and data ready
    //         while (accelerator.mpu_dmp_current_packet_size < mpu_dmp_packet_size)
    //             accelerator.mpu_dmp_current_packet_size = accelerator.mpu.getFIFOCount();

    //         uint8_t fifo_buffer[64];
    //         accelerator.mpu.getFIFOBytes(fifo_buffer, mpu_dmp_packet_size);

    //         accelerator.mpu_dmp_current_packet_size -= mpu_dmp_packet_size;

    //         Quaternion quaternion;
    //         accelerator.mpu.dmpGetQuaternion(&quaternion, fifo_buffer);
    //         VectorInt16 acceleration;
    //         accelerator.mpu.dmpGetAccel(&acceleration, fifo_buffer);
    //         VectorFloat gravity;
    //         accelerator.mpu.dmpGetGravity(&gravity, &quaternion);
    //         VectorInt16 liniar_acceleration;
    //         accelerator.mpu.dmpGetLinearAccel(&liniar_acceleration, &acceleration, &gravity);

    //         out_acceleration.x = (float)liniar_acceleration.x;
    //         out_acceleration.y = (float)liniar_acceleration.y;
    //         out_acceleration.z = (float)liniar_acceleration.z;

    //         //             float sensitivity_lsb_per_g = 16384.0f; // default to 2g
    //         // switch (config.sensitivity) {
    //         //     case ACCEL_FS::A2G:  sensitivity_lsb_per_g = 16384.0f; break;
    //         //     case ACCEL_FS::A4G:  sensitivity_lsb_per_g = 8192.0f; break;
    //         //     case ACCEL_FS::A8G:  sensitivity_lsb_per_g = 4096.0f; break;
    //         //     case ACCEL_FS::A16G: sensitivity_lsb_per_g = 2048.0f; break;
    //         // }

    //         // const float g_to_mps2 = 9.80665f;
    //         // out_acceleration.x = (liniar_acceleration.x / sensitivity_lsb_per_g) * g_to_mps2;
    //         // out_acceleration.y = (liniar_acceleration.y / sensitivity_lsb_per_g) * g_to_mps2;
    //         // out_acceleration.z = (liniar_acceleration.z / sensitivity_lsb_per_g) * g_to_mps2;

    //         return Error::None;
    //     }

    //     out_acceleration.x = 0;
    //     out_acceleration.y = 0;
    //     out_acceleration.z = 0;

    //     return Error::None;
    // }
}