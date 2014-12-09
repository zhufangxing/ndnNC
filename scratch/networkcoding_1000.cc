#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"
#include <ns3/ndnSIM/utils/tracers/ndn-cs-tracer.h>
#include <ns3/ndnSIM/utils/tracers/ndn-app-delay-tracer.h>
using namespace ns3;

int main (int argc, char *argv[])
{
  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("100Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("0.1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("100"));
  CommandLine cmd;
  cmd.Parse (argc, argv);
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topo/topo1000.txt");
  topologyReader.Read ();


  /*PointToPointHelper p2p;
  // Creating nodes
  Ptr<Node> Node1 = CreateObject<Node> ();
  Ptr<Node> Node2 = CreateObject<Node> ();
  Ptr<Node> Node3 = CreateObject<Node> ();
  Ptr<Node> Node4 = CreateObject<Node> ();
  Ptr<Node> Node5 = CreateObject<Node> ();
  Ptr<Node> Node6 = CreateObject<Node> ();
  Ptr<Node> Node7 = CreateObject<Node> ();
  p2p.Install(Node1, Node2);
  p2p.Install(Node1, Node3);
  p2p.Install(Node2, Node4);
  p2p.Install(Node2, Node6);
  p2p.Install(Node3, Node4);
  p2p.Install(Node3, Node7);
  p2p.Install(Node4, Node5);
  p2p.Install(Node5, Node6);
  p2p.Install(Node5, Node7);*/

  // Install CCNx stack on all nodes
  ndn::StackHelper ccnxHelper;
  ccnxHelper.SetDefaultRoutes(true);
  //ccnxHelper.InstallAll ();
  // Installing applications
  ccnxHelper.SetPit ("ns3::ndn::pit::Lru","MaxSize", "1000");
  ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "1000");
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
  ccnxHelper.InstallAll ();
  
  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();
  //Add /prefix origins to ndn::GlobalRouter
  Ptr<Node> Node100 = Names::Find<Node>("Node100");
  Ptr<Node> Node600 = Names::Find<Node>("Node600");
  Ptr<Node> Node700 = Names::Find<Node>("Node700");
  Ptr<Node> Node200 = Names::Find<Node>("Node200");
  Ptr<Node> Node300 = Names::Find<Node>("Node300");
  Ptr<Node> Node400 = Names::Find<Node>("Node400");
  Ptr<Node> Node500 = Names::Find<Node>("Node500");
  Ptr<Node> Node800 = Names::Find<Node>("Node800");
  Ptr<Node> Node900 = Names::Find<Node>("Node900");
  Ptr<Node> Node1000 = Names::Find<Node>("Node999");

  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node100);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node200);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node300);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node400);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node500);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node600);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node700);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node800);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node900);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node1000);
  //ccnxGlobalRoutingHelper.CalculateRoutes ();

  //ndn::AppHelper Client("ns3::ndn::ConsumerCbr");
  ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");
  Client.SetPrefix("/ndn/vod/nc");
  Client.SetAttribute("Frequency", StringValue("100"));
  Client.Install (Node100);
  Client.Install (Node200);
  Client.Install (Node300);
  Client.Install (Node400);
  Client.Install (Node600);
  Client.Install (Node700);
  Client.Install (Node800);
  Client.Install (Node900);
  Client.Install (Node1000);
  
  /*ndn::AppHelper producer("ns3::ndn::Producer");
  producer.SetPrefix("/ndn/vod/nc");
  producer.SetAttribute("PayloadSize", StringValue("1024"));
  producer.Install (one);*/
  //ccnxGlobalRoutingHelper.CalculateRoutes ();
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  
  ndn::AppHelper consumerHelper ("NetworkCodingApp");
  //ApplicationContainer app1 = consumerHelper.Install (node1); 
  //ApplicationContainer app2 = consumerHelper.Install (node2);
  //ApplicationContainer app3 = consumerHelper.Install (node3);
  //ApplicationContainer app1 = producer.Install (node1);
  consumerHelper.Install (Node500);
  ccnxGlobalRoutingHelper.CalculateRoutes ();
  //app1.Start(Seconds (0.0)); // will send out Interest, which nobody will receive (Interests generated by an app will not got back to the app)
  //app1.Stop(Seconds(20.0));
  //app6.Start (Seconds (1.0)); // will send out an Interests, which will be received and satisfied by app1
  //app7.Start (Seconds (1.0));
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > >
  AggregateTracer = ndn::L3AggregateTracer::InstallAll ("topo1000/aggregate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3RateTracer> > >
  L3RateTracer = ndn::L3RateTracer::InstallAll ("topo1000/rate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::CsTracer> > >
  aggTracers = ndn::CsTracer::InstallAll ("topo1000/cs-trace.txt", Seconds (1));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::AppDelayTracer> > >
  delayTracer = ndn::AppDelayTracer::InstallAll ("topo1000/app-delays-trace.txt");

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
  {
    std::cout << "Node #" << (*node)->GetId () << std::endl;
    (*node)->GetObject<ndn::ContentStore> ()->Print (std::cout);
      std::cout << std::endl;
  }
  Simulator::Destroy ();
  return 0;
}
