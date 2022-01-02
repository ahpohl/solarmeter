#ifndef SolarmeterEnums_h
#define SolarmeterEnums_h

enum class LogLevelEnum : unsigned char
{
  CONFIG = 0x01,
  JSON = 0x02,
  MQTT = 0x04,
  SERIAL = 0x08
};

#endif
