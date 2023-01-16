#include <Wt/WBootstrapTheme.h>

#include "waverider.h"
#include "wradio-controller.h"
#include "radio-server.h"
#include "cstreamingserver.h"
#include "string.h"
#include "tool.h"

using namespace std;
using namespace Wt;

typedef struct 
{
  string type;
  string message;
} start_logger;

unique_ptr<WApplication> createApplication(const WEnvironment& env, CRadioServer& server)
{
  auto app = make_unique<WaveriderApplication>(env, server);
  auto theme = std::make_shared<Wt::WBootstrapTheme>();
  theme->setVersion(Wt::BootstrapVersion::v3);
  app->setTheme(theme);
  //cout << "Approot: " << WApplication::appRoot() << endl;
  //app->messageResourceBundle().use(WApplication::appRoot()+"/forms/settings");
  return app;
}

void logging(string type, string message, list<start_logger*>& logger)
{
  start_logger *newlog_entry = new start_logger;
  newlog_entry->type = type;
  newlog_entry->message = message;

  logger.push_back(newlog_entry);
}

int main(int argc, char **argv)
{
  // Standard Wt Application
  /* return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
    return std::make_unique<WaveriderApplication>(env);
  }); */ 

  list<start_logger*> logger;

  // Levinstein, Russian Language Tools and so on ...
  init_tools();

  // Parse argv for http-listen
  // --approot=../approot --docroot=../docroot --http-listen 0.0.0.0:9090
  // Find out the port for http-listen
  char new_argv[255][255];
  int new_argc = 0;
  for (int i=0; i < argc; i++) 
  {
    logging("notice", "[main] Starting Server with arguments: " + string(argv[i]), logger);
    
    if (strcmp("--http-listen", argv[i]) == 0)
    {
      strcpy(new_argv[new_argc], argv[i]);
      new_argc++;

      // read IP and Port 
      char address[255];
      char ip_address[255];
      char port[255];
      strcpy(address, argv[i+1]);
      int init_size = strlen(address);
	    char delim[] = ":";

	    char *ptr = strtok(address, delim);

      int count = 0;
	    while(ptr != NULL)
	    {
        if (count == 0)
        {
          strcpy(ip_address, ptr);
          count++;
        }
        else if (count == 1)
        {
          strcpy(port, ptr);
          count++;
        }
		    //printf("'%s'\n", ptr);
		    ptr = strtok(NULL, delim);
	    }

      //cout << "IP-Address Server:" << ip_address << endl;
      //cout << "Port Server:" << port << endl;
      //Port for Streamingserver is always webservice port + 1
      int new_port = atoi(port)+1;
      char new_port_str[255];
      sprintf(new_port_str, "%d", new_port);
      strcpy(address, ip_address);
      strcat(address, ":"); 
      strcat(address, new_port_str);

      strcpy(new_argv[new_argc], address);
      new_argc++;

      break;
    }
    else
    {
      strcpy(new_argv[i],argv[i]);
      new_argc++;
    }
  }

  // Test for additional settings
  char *n_argv[new_argc];
  for (int i=0; i<new_argc; i++)
  { 
    n_argv[i] = new_argv[i];
  }
  /* for (int i=0; i<new_argc; i++)
  {
    cout << "New Argumente: " << n_argv[i] << endl;
  } */
  // Internet Radio Server
  CStreamingServer streamingServer(new_argc, n_argv, WTHTTP_CONFIGURATION);
  
  // DAB+ Radio Server
  CRadioServer radioServer(argc, argv, WTHTTP_CONFIGURATION, &streamingServer);

  for (list<start_logger*>::iterator it = logger.begin(); it != logger.end(); ++it)
  {
    start_logger* s = *it;
    radioServer.log(s->type, s->message);
  }

  radioServer.addEntryPoint(EntryPointType::Application, bind(createApplication, placeholders::_1, ref(radioServer)));

  // auch der Streaming-Server schickt die dekodierten Frames an den RadioController (onNewAudio)
  streamingServer.setRadioController(radioServer.getRadioController());
  streamingServer.setLogger(radioServer.logger());
  streamingServer.start();

  if (radioServer.start()) 
  {
    //int sig = WServer::waitForShutdown();
    try 
    {
      int sig = radioServer.waitForShutdown();

      cerr << "sig" << " : Signal returned" << endl;
    }
    catch (WServer::Exception e)
    {
      cerr << e.what() << endl;
    }
    
    //streamingServer.stop();
    //radioServer.stop();  
  }  
}