#ifndef Solarmeter_h
#define Solarmeter_h
#include <ABBAurora.h>
#include "SolarmeterMqtt.h"
#include "SolarmeterConfig.h"

class Solarmeter
{
  static const std::set<std::string> ValidKeys;

private:
  ABBAurora *Inverter;
  SolarmeterMqtt *Mqtt;
  SolarmeterConfig *Cfg;
  std::stringstream Payload;
  std::string Config;
  std::string ErrorMessage;
  bool Log;

  template <typename T>
  T StringTo(const std::string &str) const;
 
public:
  Solarmeter(const bool &log);
  ~Solarmeter(void);
  bool Setup(const std::string &config);
  bool Receive(void);
  bool Publish(void);
  std::string GetErrorMessage(void) const;
  std::string GetPayload(void) const;
  int GetState(void);
  
  struct Datagram
  {
    std::string SerialNum;    // Serial number
    std::string PartNum;      // Part number
    std::string MfgDate;      // Manufacturing date
    std::string Firmware;     // Firmware version
    std::string InverterType; // Inverter type indoor/outdoor
    std::string GridStandard; // Grid standard
    float VoltageP1;          // Voltage pin 1 [V]
    float CurrentP1;          // Current pin 1 [A]
    float PowerP1;            // Power pin 1 [W]
    float VoltageP2;          // Voltage pin 2 [V]
    float CurrentP2;          // Current pin 2 [A]
    float PowerP2;            // Power pin 2 [W]
    float GridVoltage;        // Grid voltage [V]
    float GridCurrent;        // Grid current [A]
    float GridPower;          // Grid power [W]
    float Frequency;          // Grid frequency [Hz]
    float Efficiency;         // AC/DC conversion efficiency [%]
    float InverterTemp;       // Inverter temperature [°C]
    float BoosterTemp;        // Booster temperature [°C]
    float RIso;               // Isolation resistance [MOhm] 
    float TotalEnergy;        // Lifetime total energy [kWh]
    float PaymentKwh;         // Payment per kWh
  } Datagram;

  struct State
  {
    std::string GlobalState;  // Global state
    std::string InverterState;// Inverter state
    std::string Channel1State;// Channel 1 state
    std::string Channel2State;// Channel 2 state
    std::string AlarmState;   // Alarm state
  };

  bool operator==(const State& lhs, const State& rhs);
};

#endif
