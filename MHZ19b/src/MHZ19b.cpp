#include "MHZ19b.h"

MHZ19b::MHZ19b(SoftwareSerial *p_serial)
{
	ss = p_serial;
}

MHZ19b::MHZ19b(HardwareSerial *p_serial)
{
	hs = p_serial;
}

MHZ19b::~MHZ19b()
{
	hs = nullptr;
	ss = nullptr;
}

void MHZ19b::init() {
    MHZ19b::current_action = MHZ19b::read_co2;
}

void MHZ19b::set_channel(vid_channel p_channel) {
    current_channel = p_channel;
}

byte MHZ19b::calc_crc(byte *p_cmd) {
  byte crc = 0;
  for (int i = 1; i < cmd_len - 1; i++) crc += p_cmd[i];
  crc = 255 - crc;
  crc++;
  return crc;
}

bool MHZ19b::check_crc(byte *p_cmd) {
  return (p_cmd[0] == start_byte && p_cmd[1] == current_action && p_cmd[8] == calc_crc(p_cmd));
}

byte* MHZ19b::get_cmd() {
    request[0] = start_byte;
    request[1] = sensor;
    request[2] = current_action;
    for(int i = 3; i < cmd_len - 1; i++) request[i] = def_cmd;
    request[cmd_len-1] = calc_crc(request);
    return request;
}

void MHZ19b::send_command(byte *p_cmd) {
    if(ss) {
       ss->write(p_cmd, cmd_len);
       ss->flush();
    }else if(hs) {
       hs->write(p_cmd, cmd_len);
       hs->flush();
    }
}

byte* MHZ19b::receive_data() {
    if(ss) {
        ss->readBytes(response, cmd_len);
    }else if(hs) {
        hs->readBytes(response, cmd_len);
    }
    response[cmd_len - 1] = calc_crc(response);
    //if(check_crc(response)) throw "Check CRC failed";

    return response;
}

int MHZ19b::get_pwm_data() {
    int th = pulseIn(0, HIGH, 104000) / 1000 - 4;
    int tl = 1004 - th;
    MHZ19b::co2 = 2000 * (th - 2) / (th + tl - 4);

    return MHZ19b::co2;
}

int MHZ19b::get_uart_data() {
   if(!request) {
      if(current_action != MHZ19b::read_co2) set_action(MHZ19b::read_co2);
      get_cmd();
    }
    send_command(request);
    receive_data();
  
    int responseHigh = static_cast<int>(response[2]);
    int responseLow = static_cast<int>(response[3]);
    MHZ19b::co2 = (256*responseHigh) + responseLow;

    return MHZ19b::co2;
}


int MHZ19b::get_co2() {
   switch(current_channel) {
       case uart:
            MHZ19b::co2 = get_uart_data();
            break;
        case pwm:
            MHZ19b::co2 = get_pwm_data();
            break;
   }
       
    return MHZ19b::co2;
}

void MHZ19b::set_action(command p_cmd) {
    current_action = p_cmd;
}

int MHZ19b::get_temperature() {
  if(current_channel != uart) throw "Selected not correct channel";
  if(current_action != read_co2) throw "Selected not correct action";

  if(!request) {
      get_cmd();
  }
  send_command(request);
  receive_data();

   MHZ19b::temperature = static_cast<int>(response[4]) - 40;
   return MHZ19b::temperature;
}

air_quality MHZ19b::get_quality_air() {
    if(MHZ19b::co2 < 800) {
        return high_air;
    } else if((MHZ19b::co2 >= 800 && (MHZ19b::co2 < 1000))) {
        return medium_air;
    }else if((MHZ19b::co2 >= 1000 && (MHZ19b::co2 < 1400))) {
        return low_air;
    }else if((MHZ19b::co2 >= 1400)) {
        return bad_air;
    }
}

void MHZ19b::set_autocalibration(bool p_in) {
    current_action = auto_calibration;
    get_cmd();
    if(p_in){
        request[4] = 0xA0;
    }else {
        request[4] = 0x00;
    }
    send_command(request);
}

void MHZ19b::calibrate_zero() {
    current_action = calibrate_sensor_zero;
    send_command(get_cmd());
}