#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int 
main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Creating nodes
  Ptr<Node> node = CreateObject<Node> ();

  // Install CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.InstallAll ();

  // Installing applications

  // Consumer
  ndn::AppHelper consumerHelper ("CustomApp");
  ApplicationContainer app1 = consumerHelper.Install (node); 
  ApplicationContainer app2 = consumerHelper.Install (node);

  app1.Start (Seconds (1.0)); // will send out Interest, which nobody will receive (Interests generated by an app will not got back to the app)
  app2.Start (Seconds (2.0)); // will send out an Interests, which will be received and satisfied by app1
  
  Simulator::Stop (Seconds (3.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
