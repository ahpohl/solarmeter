#ifndef SolarmeterMqtt_h
#define SolarmeterMqtt_h
#include <mosquitto.h>
#include <string>

class SolarmeterMqtt {
  static void OnConnectCallbackWrapper(struct mosquitto *mosq, void *obj,
                                       int connack_code);
  static void LogCallbackWrapper(struct mosquitto *mosq, void *obj, int level,
                                 const char *str);

private:
  struct mosquitto *Mosq;
  void OnConnectCallback(struct mosquitto *mosq, void *obj, int connack_code);
  void LogCallback(struct mosquitto *mosq, void *obj, int level,
                   const char *str);
  std::string ErrorMessage;
  volatile bool IsConnected;
  volatile bool NotifyOnlineFlag;
  unsigned char Log;

public:
  SolarmeterMqtt(void);
  ~SolarmeterMqtt(void);
  void SetLogLevel(const unsigned char &log_level);
  bool Begin(void);
  bool Connect(const std::string &host, const int &port, const int &keepalive);
  bool SetUserPassAuth(const std::string &user, const std::string &pass);
  bool SetLastWillTestament(const std::string &message,
                            const std::string &topic, const int &qos,
                            const bool &retain);
  bool SetTlsConnection(const std::string &cafile, const std::string &capath);
  bool PublishMessage(const std::string &message, const std::string &topic,
                      const int &qos, const bool &retain);
  std::string GetErrorMessage(void) const;
  bool GetConnectStatus(void) const;
  bool GetNotifyOnlineFlag(void) const;
  void SetNotifyOnlineFlag(const bool &flag);
};

#endif
