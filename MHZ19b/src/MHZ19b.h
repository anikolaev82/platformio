#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

enum air_quality {
            high_air   = 0,
            medium_air = 1,
            low_air    = 2,
            bad_air    = 3
        };
enum vid_channel {
    pwm,
    uart
};

 class MHZ19b {
    public:
        MHZ19b(SoftwareSerial *p_serial);
        MHZ19b(HardwareSerial *p_serial);
        ~MHZ19b();
        
        void init();
        void set_channel(vid_channel p_channel);

        int get_temperature();
        int get_co2();
        air_quality get_quality_air();

        void calibrate_zero();
        void set_autocalibration(bool p_on);
        
        void set_port_speed(int p_port_speed);

    private:
        enum command {
                        read_co2              = 0x86,
                        calibrate_sensor_zero = 0x87,
                        calibrate_sensor_span = 0x88,
                        auto_calibration      = 0x79,
                        detect_range          = 0x99
        };        

        SoftwareSerial *ss = nullptr;
        HardwareSerial *hs = nullptr;

        command current_action;
        vid_channel current_channel;
        byte response[9], request[9];
        
        int co2;
        int temperature;

        int port_speed = 9600;
        int const cmd_len = 9;
        byte const start_byte = 0xFF;
        byte const sensor = 0x01;
        byte const def_cmd = 0x00;

        int get_pwm_data();
        int get_uart_data();
        void set_action(command p);
        bool check_crc(byte *p_crc);
        byte calc_crc(byte *p_cmd);
        void send_command(byte *p_cmd);
        byte* receive_data();
        byte* get_cmd();
};