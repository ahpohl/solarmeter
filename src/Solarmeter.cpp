#include <iostream>
#include <cstring>
#include <charconv>
#include <chrono>
#include <thread>
#include "Solarmeter.h"

const std::set<std::string> Solarmeter::ValidKeys {"mqtt_broker", "mqtt_password", "mqtt_port", "mqtt_topic", "mqtt_user", "mqtt_tls_cafile", "mqtt_tls_capath", "payment_kwh", "serial_device"};

Solarmeter::Solarmeter(const bool &log): Log(log)
{
  Inverter = new ABBAurora();;
  Mqtt = new SolarmeterMqtt(Log);
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
  if (Log)
  {
    Cfg->ShowConfig();
  }
  if (!Cfg->ValidateKeys(Solarmeter::ValidKeys))
  {
    ErrorMessage = Cfg->GetErrorMessage();
    return false;
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
  if (!Mqtt->SetLastWillTestament("offline", Cfg->GetValue("mqtt_topic") + "/status", 1, true))
  {
    ErrorMessage = Mqtt->GetErrorMessage();
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
  if (!Mqtt->Connect(Cfg->GetValue("mqtt_broker"), StringTo<double>(Cfg->GetValue("mqtt_port")), 60))
  {
    ErrorMessage = Mqtt->GetErrorMessage();
    return false;
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  if (Mqtt->GetConnectStatus())
  {
    std::cout << "Solarmeter is online." << std::endl;
  }
  if (!Mqtt->PublishMessage("online", Cfg->GetValue("mqtt_topic") + "/status", 1, true))
  {
    ErrorMessage = Mqtt->GetErrorMessage();
    return false;
  }

  return true;
}

bool Solarmeter::Receive(void)
{
  ABBAurora::State state;
  if (!Inverter->ReadState(state))
  {
    ErrorMessage = Inverter->GetErrorMessage();
    return false;
  }
  Datagram.Status = state.GlobalState;

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

  return true;
}

/*
bool Solarmeter::Publish(void)
{
  unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
  Payload.str(std::string());

  Payload << "[{"
    << "\"lifetime\":" << Datagram.SensorTime << "," 
    << "\"energy\":" << RemoveLeading(Datagram.Energy, '0') << ","
    << "\"power\":" << RemoveLeading(Datagram.Power, '0') << ","
    << "\"power_l1\":" << RemoveLeading(Datagram.PowerL1, '0') << ","
    << "\"power_l2\":" << RemoveLeading(Datagram.PowerL2, '0') << ","
    << "\"power_l3\":" << RemoveLeading(Datagram.PowerL3, '0') << ","
    << "\"voltage_l1\":" << RemoveLeading(Datagram.VoltageL1, '0') << ","
    << "\"voltage_l2\":" << RemoveLeading(Datagram.VoltageL2, '0') << ","
    << "\"voltage_l3\":" << RemoveLeading(Datagram.VoltageL3, '0') << ","
    << "\"status\":\"" << Datagram.Status << "\"" << ","
    << "\"rate\":"  << Cfg->GetValue("plan_basic_rate") << ","
    << "\"price\":" << Cfg->GetValue("plan_price_kwh") << ","
    << "\"time\":" << now
    << "},{"
    << "\"serial\":\"" << Datagram.SerialNum << "\","
    << "\"custom_id\":\"" << Datagram.CustomId << "\","
    << "\"device_id\":\"" << Datagram.DeviceId << "\""
    << "}]";

  static bool last_connect_status = true;
  if (Mqtt->GetConnectStatus())
  {
    if (!(Mqtt->PublishMessage(Payload.str(), Cfg->GetValue("mqtt_topic") + "/state", 0, false)))
    {
      ErrorMessage = Mqtt->GetErrorMessage();
      return false;
    }
    if (!last_connect_status)
    {
      if (!(Mqtt->PublishMessage("online", Cfg->GetValue("mqtt_topic") + "/status", 1, true)))
      {
        ErrorMessage = Mqtt->GetErrorMessage();
        return false;
      }
      std::cout << "Solarmeter is online." << std::endl;
    }
  }
  last_connect_status = Mqtt->GetConnectStatus();
 
  return true;
}
*/

std::string Solarmeter::GetErrorMessage(void) const
{
  return ErrorMessage;
}

std::string Solarmeter::GetPayload(void) const
{
  return Payload.str();
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
