#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  CommandLine cmd;
  cmd.Parse (argc, argv);
  // Creating nodes
  Ptr<Node> node = CreateObject<Node> ();
  Ptr<Node> node1 = CreateObject<Node> ();
  // Install CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  //ccnxHelper.InstallAll ();
  // Installing applications
  cnxHelper.SetPit ("ns3::ndn::pit::Lru","MaxSize", "0");
  ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "1000");
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
  ccnxHelper.InstallAll ();
  
  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();
  //Add /prefix origins to ndn::GlobalRouter
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/nc/nc", node);
  ccnxGlobalRoutingHelper.CalculateRoutes ();
  // Consumer
  ndn::AppHelper consumerHelper ("NetworkCodingApp");
  ApplicationContainer app1 = consumerHelper.Install (node); 
  ApplicationContainer app2 = consumerHelper.Install (node);
  ApplicationContainer app3 = consumerHelper.Install (node1);
  app1.Start (Seconds (1.0)); // will send out Interest, which nobody will receive (Interests generated by an app will not got back to the app)
  app2.Start (Seconds (1.0)); // will send out an Interests, which will be received and satisfied by app1
  app3.Start (Seconds (1.0));
  Simulator::Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
