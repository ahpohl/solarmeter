#include <iostream>
#include <iomanip>
#include <memory>
#include <getopt.h>
#include <csignal>
#include <thread>
#include <chrono>
#include <Solarmeter.h>

volatile sig_atomic_t shutdown = false;

void sig_handler(int)
{
  shutdown = true;
}

int main(int argc, char* argv[])
{
  struct sigaction action;
  action.sa_handler = sig_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
  
  int verbose_level = 0;
  bool version = false;
  bool help = false;
  std::string config;

  const struct option longOpts[] = {
    { "help", no_argument, nullptr, 'h' },
    { "version", no_argument, nullptr, 'V' },
    { "verbose", no_argument, nullptr, 'v' },
    { "config", required_argument, nullptr, 'c' },
    { nullptr, 0, nullptr, 0 }
  };

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
    case 'v':
      ++verbose_level;
      break;
    case 'c':
      config = optarg;
      break;
    default:
      break;
    }

  } while (opt != -1);

  if (help)
  {
    std::cout << "Solarmeter " << VERSION_TAG << std::endl;
    std::cout << std::endl << "Usage: " << argv[0] << " [-vv] -c [file]" << std::endl;
    std::cout << "\n\
  -h --help         Show help message\n\
  -V --version      Show build info\n\
  -v --verbose      Set verbose output level\n\
  -c --config       Set config file"
    << std::endl << std::endl;
    return EXIT_SUCCESS;
  }

  if (version)
  {
    std::cout << "Version " << VERSION_TAG 
      << " (" << VERSION_BUILD << ") built " 
      << VERSION_BUILD_DATE 
      << " by " << VERSION_BUILD_MACHINE << std::endl;
    return EXIT_SUCCESS;
  }

  std::cout << "Solarmeter " << VERSION_TAG
    << " (" << VERSION_BUILD << ")" << std::endl;

  bool log = (verbose_level == 2) ? true : false;
  std::unique_ptr<Solarmeter> meter(new Solarmeter(log));
  
  if (!meter->Setup(config))
  {
    std::cout << meter->GetErrorMessage() << std::endl;
    return EXIT_FAILURE;
  }

  bool is_active = true;

  while (shutdown == false)
  {
    is_active = (meter->IsRunning() && meter->IsInput());
    if (is_active)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::seconds(10));
    }
	  if (!meter->Receive())
	  {
      if (is_active)
      {
	      std::cout << meter->GetErrorMessage() << std::endl;
      }
      continue;
 	  }
    if (!meter->Publish())
    {
      std::cout << meter->GetErrorMessage() << std::endl;
    }
    if (verbose_level == 1)
    {
      std::cout << meter->GetPayload() << std::endl;
    }
  }

  return EXIT_SUCCESS;
}
