#include <Wt/WBootstrapTheme.h>

#include "waverider.h"
#include "wradio-controller.h"
#include "radio-server.h"

using namespace std;
using namespace Wt;

unique_ptr<WApplication> createApplication(const WEnvironment& env, CRadioServer& server)
{
  auto app = make_unique<WaveriderApplication>(env, server);
  auto theme = std::make_shared<Wt::WBootstrapTheme>();
  theme->setVersion(Wt::BootstrapVersion::v3);
  app->setTheme(theme);
  return app;
}

int main(int argc, char **argv)
{
  // Standard
  /* return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<WaveriderApplication>(env);
  }); */ 

  // Server
  CRadioServer radioServer(argc, argv, WTHTTP_CONFIGURATION);

  radioServer.addEntryPoint(EntryPointType::Application, bind(createApplication, placeholders::_1, ref(radioServer)));
  
  if (radioServer.start()) 
  {
    int sig = WServer::waitForShutdown();
    radioServer.stop();  
  }  
}