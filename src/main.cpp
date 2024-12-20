#include <Solarmeter.h>
#include <chrono>
#include <csignal>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

volatile sig_atomic_t shutdown = false;

void sig_handler(int) { shutdown = true; }

int main(int argc, char *argv[]) {
  struct sigaction action;
  action.sa_handler = sig_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);

  bool version = false;
  bool help = false;
  std::string config;

  const struct option longOpts[] = {{"help", no_argument, nullptr, 'h'},
                                    {"version", no_argument, nullptr, 'V'},
                                    {"config", required_argument, nullptr, 'c'},
                                    {nullptr, 0, nullptr, 0}};

  const char optString[] = "hVvc:";
  int opt = 0;
  int longIndex = 0;

  do {
    opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
    switch (opt) {
    case 'h':
      help = true;
      break;
    case 'V':
      version = true;
      break;
    case 'c':
      config = optarg;
      break;
    default:
      break;
    }

  } while (opt != -1);

  if (help) {
    std::cout << "Solarmeter " << VERSION_TAG << std::endl;
    std::cout << std::endl
              << "Usage: " << argv[0] << " [-vv] -c [file]" << std::endl;
    std::cout << "\n\
  -h --help         Show help message\n\
  -V --version      Show build info\n\
  -c --config       Set config file"
              << std::endl
              << std::endl;
    return EXIT_SUCCESS;
  }

  if (version) {
    std::cout << "Version " << VERSION_TAG << " (" << VERSION_BUILD
              << ") built " << VERSION_BUILD_DATE << " by "
              << VERSION_BUILD_MACHINE << std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Solarmeter " << VERSION_TAG << " (" << VERSION_BUILD << ")"
            << std::endl;

  std::unique_ptr<Solarmeter> meter(new Solarmeter());

  if (!meter->Setup(config)) {
    std::cout << meter->GetErrorMessage() << std::endl;
    return EXIT_FAILURE;
  }

  static int timeout = 0;

  while (shutdown == false) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!meter->Receive()) {
      if (timeout < 5) {
        std::cout << meter->GetErrorMessage() << std::endl;
        ++timeout;
      }
      continue;
    } else {
      timeout = 0;
    }
    if (!meter->Publish()) {
      std::cout << meter->GetErrorMessage() << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
