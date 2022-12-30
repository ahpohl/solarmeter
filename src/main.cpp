#include <iostream>
#include <getopt.h>
#include <csignal>
#include <future>

#include <Solarmeter.h>

int main(int argc, char* argv[])
{
  bool version = false;
  bool help = false;
  std::string config;

  const struct option longOpts[] = {
    { "help", no_argument, nullptr, 'h' },
    { "version", no_argument, nullptr, 'V' },
    { "config", required_argument, nullptr, 'c' },
    { nullptr, 0, nullptr, 0 }
  };

  const char optString[] = "hVc:";
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

  if (help)
  {
    std::cout << "Solarmeter " << VERSION_TAG << std::endl;
    std::cout << std::endl << "Usage: " << argv[0] << " [-vv] -c [file]" << std::endl;
    std::cout << "\n\
  -h --help         Show help message\n\
  -V --version      Show build info\n\
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

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGINT);
  sigaddset(&sigset, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

  std::atomic<bool> shutdown_requested(false);
  std::mutex cv_mutex;
  std::condition_variable cv;

  auto signal_handler = [&]()
  {
    int signum = 0;
    sigwait(&sigset, &signum);
    shutdown_requested.store(true);
    cv.notify_all();
    return signum;
  }; 

  auto ft_signal_handler = std::async(std::launch::async, signal_handler);
 
  std::unique_ptr<Solarmeter> meter(new Solarmeter());
  if (!meter->Setup(config))
  {
    std::cout << meter->GetErrorMessage() << std::endl;
    return EXIT_FAILURE;
  }

  auto worker = [&]()
  {
    int timeout = 0;

    while (shutdown_requested.load() == false)
    {
      std::unique_lock lock(cv_mutex);
	    if (!meter->Receive())
	    {
        if (timeout < 5)
        {
	        std::cout << meter->GetErrorMessage() << std::endl;
          ++timeout;
        }
        continue;
 	    }
      else
      {
        timeout = 0;
      }
      if (!meter->Publish())
      {
        std::cout << meter->GetErrorMessage() << std::endl;
      }
      cv.wait_for(lock,
                  std::chrono::seconds(1),
                  [&]() { return shutdown_requested.load(); });
    }
    return shutdown_requested.load();
  };
  
  auto worker_thread = (std::async(std::launch::async, worker));

  return EXIT_SUCCESS;
}
