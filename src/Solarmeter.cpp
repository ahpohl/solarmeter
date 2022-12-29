#include <iostream>
#include <iomanip>
#include <cstring>
#include <charconv>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <vector>
#include "Solarmeter.h"

const std::set<std::string> Solarmeter::ValidKeys {"log_level", "mqtt_broker", "mqtt_password", "mqtt_port", "mqtt_topic", "mqtt_user", "mqtt_tls_cafile", "mqtt_tls_capath", "payment_kwh", "serial_device"};

Solarmeter::Solarmeter(void)
{
  Inverter = new ABBAurora();;
  Mqtt = new SolarmeterMqtt();
  Cfg = new SolarmeterConfig();
}

Solarmeter::~Solarmeter(void)
{
  if (Mqtt->GetConnectStatus())
  {
    Mqtt->PublishMessage("offline", Cfg->GetValue("mqtt_topic") + "/status", 1, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  if (Mqtt) { delete Mqtt; }
  if (Inverter) { delete Inverter; }
  if (Cfg) { delete Cfg; };
}

bool Solarmeter::Setup(const std::string &config)
{
  if (!Cfg->Begin(config))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  if (!Cfg->ValidateKeys(Solarmeter::ValidKeys))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  this->SetLogLevel();
  Inverter->SetLogLevel(Log);
  Mqtt->SetLogLevel(Log);
  if (Log & static_cast<unsigned char>(LogLevelEnum::CONFIG))
  {
    Cfg->ShowConfig();
  }
  if (!(Cfg->KeyExists("payment_kwh")))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  if (!(Cfg->KeyExists("serial_device")))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  if (!Inverter->Setup(Cfg->GetValue("serial_device")))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadCumulatedEnergy(Datagram.TotalEnergy, CumulatedEnergyEnum::LIFETIME_TOTAL))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  std::cout << "Current meter reading: " << Datagram.TotalEnergy << " kWh" << std::endl;
  
  if (!Mqtt->Begin())
  {
    ErrorMessage = Mqtt->GetErrorMessage();
    return false;
  }
  if (!(Cfg->KeyExists("mqtt_topic")))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  if ((Cfg->KeyExists("mqtt_user") && Cfg->KeyExists("mqtt_password")))
  {
    if (!Mqtt->SetUserPassAuth(Cfg->GetValue("mqtt_user"), Cfg->GetValue("mqtt_password")))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
  }
  if (Cfg->KeyExists("mqtt_tls_cafile") || Cfg->KeyExists("mqtt_tls_capath"))
  {
    if (!Mqtt->SetTlsConnection(Cfg->GetValue("mqtt_tls_cafile"), Cfg->GetValue("mqtt_tls_capath")))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
  }
  if (!(Cfg->KeyExists("mqtt_broker")) || !(Cfg->KeyExists("mqtt_port")) )
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
  }
  if (!Mqtt->SetLastWillTestament("offline", Cfg->GetValue("mqtt_topic") + "/status", 1, true))
  {
    ErrorMessage = Mqtt->GetErrorMessage();
    return false;
  }
  if (!Mqtt->Connect(Cfg->GetValue("mqtt_broker"), StringTo<double>(Cfg->GetValue("mqtt_port")), 60))
  {
    ErrorMessage = Mqtt->GetErrorMessage();
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  
  return true;
}

bool Solarmeter::Receive(void)
{
  if (!Inverter->ReadState(State))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadPartNumber(Datagram.PartNum))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadSerialNumber(Datagram.SerialNum))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  ABBAurora::FirmwareRelease firmware;
  if (!Inverter->ReadFirmwareRelease(firmware))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  Datagram.Firmware = firmware.Release;
  
  ABBAurora::ManufacturingDate mfg_date;
  if (!Inverter->ReadManufacturingDate(mfg_date))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  Datagram.MfgDate = std::string("Year ") + mfg_date.Year + " Week " + mfg_date.Week;

  ABBAurora::Version version;
  if (!Inverter->ReadVersion(version))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  Datagram.InverterType = version.Par1;
  Datagram.GridStandard = version.Par2;

  if (!Inverter->ReadCumulatedEnergy(Datagram.TotalEnergy, CumulatedEnergyEnum::LIFETIME_TOTAL))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.VoltageP1, DspValueEnum::V_IN_1))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.CurrentP1, DspValueEnum::I_IN_1))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.PowerP1, DspValueEnum::POWER_IN_1))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.VoltageP2, DspValueEnum::V_IN_2))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.CurrentP2, DspValueEnum::I_IN_2))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.PowerP2, DspValueEnum::POWER_IN_2))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.GridVoltage, DspValueEnum::GRID_VOLTAGE))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.GridCurrent, DspValueEnum::GRID_CURRENT))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.GridPower, DspValueEnum::GRID_POWER))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.Frequency, DspValueEnum::FREQUENCY))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.InverterTemp, DspValueEnum::TEMPERATURE_INVERTER))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.BoosterTemp, DspValueEnum::TEMPERATURE_BOOSTER))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  if (!Inverter->ReadDspValue(Datagram.RIso, DspValueEnum::ISOLATION_RESISTANCE))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  try
  {
    float denominator = Datagram.PowerP1 + Datagram.PowerP2;
    if (denominator == 0)
    {
      throw std::runtime_error("Math error: Attempted to divide by Zero");
    } 
    Datagram.Efficiency = Datagram.GridPower / denominator * 100.0f;
  }
  catch (std::runtime_error& e)
  {
    ErrorMessage = e.what();
    return false;
  }
  return true;
}

bool Solarmeter::Publish(void)
{
  unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  
  std::ios::fmtflags old_settings = Payload.flags();
  Payload.str(std::string());
  Payload.setf(std::ios::fixed, std::ios::floatfield);

  Payload << "[{"
    << "\"time\":" << now << ","
    << "\"total_energy\":" << std::setprecision(2) << Datagram.TotalEnergy << "," 
    << "\"voltage_p1\":" << std::setprecision(2) << Datagram.VoltageP1 << ","
    << "\"current_p1\":" << std::setprecision(5) << Datagram.CurrentP1 << ","
    << "\"power_p1\":" << std::setprecision(2) << Datagram.PowerP1 << ","
    << "\"voltage_p2\":" << std::setprecision(2) << Datagram.VoltageP2 << ","
    << "\"current_p2\":" << std::setprecision(5) << Datagram.CurrentP2 << ","
    << "\"power_p2\":" << std::setprecision(2) << Datagram.PowerP2 << ","
    << "\"grid_voltage\":" << std::setprecision(2) << Datagram.GridVoltage << ","
    << "\"grid_current\":" << std::setprecision(5) << Datagram.GridCurrent << ","
    << "\"grid_power\":" << std::setprecision(2) << Datagram.GridPower << ","
    << "\"frequency\":" << std::setprecision(3) << Datagram.Frequency << ","
    << "\"efficiency\":" << std::setprecision(2) << Datagram.Efficiency << ","
    << "\"inverter_temp\":" << std::setprecision(2) << Datagram.InverterTemp << ","
    << "\"booster_temp\":" << std::setprecision(2) << Datagram.BoosterTemp << ","
    << "\"r_iso\":" << std::setprecision(3) << Datagram.RIso << ","
    << "\"payment\":" << Cfg->GetValue("payment_kwh")
    << "},{"
    << "\"serial_num\":\"" << Datagram.SerialNum << "\","
    << "\"part_num\":\"" << Datagram.PartNum << "\","
    << "\"mfg_date\":\"" << Datagram.MfgDate << "\","
    << "\"firmware\":\"" << Datagram.Firmware << "\","
    << "\"inverter_type\":\"" << Datagram.InverterType << "\"," 
    << "\"grid_standard\":\"" << Datagram.GridStandard << "\""
    << "}]";

  std::ostringstream oss;
  oss << "[{"
    << "\"global_state\":\"" << State.GlobalState << "\"" << ","
    << "\"inverter_state\":\"" << State.InverterState << "\"" << ","
    << "\"ch1_state\":\"" << State.Channel1State << "\"" << ","
    << "\"ch2_state\":\"" << State.Channel2State << "\"" << ","
    << "\"alarm_state\":\"" << State.AlarmState << "\"" << "}]";

  if (Mqtt->GetNotifyOnlineFlag())
  {
    std::cout << "Solarmeter is online." << std::endl;
    if (!Mqtt->PublishMessage("online", Cfg->GetValue("mqtt_topic") + "/status", 1, true))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
  }

  static ABBAurora::State previous_state;
  if ( Mqtt->GetNotifyOnlineFlag() || 
       (!((previous_state.GlobalState == State.GlobalState) &&
       (previous_state.InverterState == State.InverterState) &&
       (previous_state.Channel1State == State.Channel1State) &&
       (previous_state.Channel2State == State.Channel2State) &&
       (previous_state.AlarmState == State.AlarmState))) )
  {
    if (!(Mqtt->PublishMessage(oss.str(), Cfg->GetValue("mqtt_topic") + "/state", 0, false)))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
    previous_state = State;
  }
  if (Mqtt->GetConnectStatus())
  {
    if (!(Mqtt->PublishMessage(Payload.str(), Cfg->GetValue("mqtt_topic") + "/live", 0, false)))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
  }
  Mqtt->SetNotifyOnlineFlag(false);

  if (Log & static_cast<unsigned char>(LogLevelEnum::JSON))
  {
    std::cout << Payload.str() << std::endl;
    std::cout << oss.str() << std::endl;
  }
  Payload.flags(old_settings);
  
  return true;
}

std::string Solarmeter::GetErrorMessage(void) const
{
  return ErrorMessage;
}

void Solarmeter::SetLogLevel(void)
{
  if (Cfg->KeyExists("log_level"))
  {
    std::string line = Cfg->GetValue("log_level");
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> log_level;

    while(std::getline(iss, token, ','))
    {
      log_level.push_back(token);
    }
    for (auto it = log_level.cbegin(); it != log_level.cend(); ++it)
    {
      if (!(*it).compare("config"))
      {
        Log |= static_cast<unsigned char>(LogLevelEnum::CONFIG);
      }
      else if (!(*it).compare("json"))
      {
        Log |= static_cast<unsigned char>(LogLevelEnum::JSON);
      }
      else if (!(*it).compare("mosquitto"))
      {
        Log |= static_cast<unsigned char>(LogLevelEnum::MQTT);
      }
      else if (!(*it).compare("serial"))
      {
        Log |= static_cast<unsigned char>(LogLevelEnum::SERIAL);
      }
    }
  }
  else
  {
    Log = 0;
  }
}

unsigned char Solarmeter::GetLogLevel(void) const
{
  return Log;
}

template <typename T>
T Solarmeter::StringTo(const std::string &str) const
{
  T value;
  std::istringstream iss(str);
  iss >> value;
  if (iss.fail())
  {
    return T();
  }
  return value;
}
