#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/point-to-point-module.h"
#include <ns3/ndnSIM/utils/tracers/ndn-cs-tracer.h>
#include <ns3/ndnSIM/utils/tracers/ndn-app-delay-tracer.h>
#include <iostream>
using namespace std;
using namespace ns3;
//namespace std{
//namespace ns3{
int main (int argc, char *argv[])
{
//std:int a;  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
  // setting default parameters for PointToPoint links and channels
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("100Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("100"));
  CommandLine cmd;
  cmd.Parse (argc, argv);
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topo/topo100.txt");
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
 // ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
  ccnxHelper.InstallAll ();
 
  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();
  //Add /prefix origins to ndn::GlobalRouter
  Ptr<Node> Node10 = Names::Find<Node>("10");
  Ptr<Node> Node60 = Names::Find<Node>("60");
  Ptr<Node> Node70 = Names::Find<Node>("70");
  Ptr<Node> Node20 = Names::Find<Node>("20");
  Ptr<Node> Node30 = Names::Find<Node>("30");
  Ptr<Node> Node40 = Names::Find<Node>("40");
  Ptr<Node> Node50 = Names::Find<Node>("50");
  Ptr<Node> Node80 = Names::Find<Node>("80");
  Ptr<Node> Node90 = Names::Find<Node>("90");
  Ptr<Node> Node0 = Names::Find<Node>("0");
Ptr<Node> Node13 = Names::Find<Node>("13");
  Ptr<Node> Node63 = Names::Find<Node>("63");
  Ptr<Node> Node73 = Names::Find<Node>("73");
  Ptr<Node> Node23 = Names::Find<Node>("23");
  Ptr<Node> Node33 = Names::Find<Node>("33");
  Ptr<Node> Node43 = Names::Find<Node>("43");
  Ptr<Node> Node53 = Names::Find<Node>("53");
  Ptr<Node> Node83 = Names::Find<Node>("83");
  Ptr<Node> Node93 = Names::Find<Node>("93");
  Ptr<Node> Node3 = Names::Find<Node>("3");
Ptr<Node> Node16 = Names::Find<Node>("16");
  Ptr<Node> Node66 = Names::Find<Node>("66");
  Ptr<Node> Node76 = Names::Find<Node>("76");
  Ptr<Node> Node26 = Names::Find<Node>("26");
  Ptr<Node> Node36 = Names::Find<Node>("36");
  Ptr<Node> Node46 = Names::Find<Node>("46");
  Ptr<Node> Node56 = Names::Find<Node>("56");
  Ptr<Node> Node86 = Names::Find<Node>("86");
  Ptr<Node> Node96 = Names::Find<Node>("96");
  Ptr<Node> Node6 = Names::Find<Node>("6");

  Ptr<Node> Node48 = Names::Find<Node>("48");

  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node10);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node20);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node30);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node40);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node50);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node60);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node70);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node80);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node90);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node0);

  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node13);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node23);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node33);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node43);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node53);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node63);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node73);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node83);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node93);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node3);

  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node16);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node26);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node36);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node46);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node56);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node66);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node76);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node86);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node96);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node6);
 
 ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node48);
  //ccnxGlobalRoutingHelper.CalculateRoutes ();

  ndn::AppHelper Client("ns3::ndn::ConsumerCbr");
  //ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");
  Client.SetPrefix("/ndn/vod/nc");
  Client.SetAttribute("Frequency", StringValue("20"));

  Client.Install (Node10);
  Client.Install (Node20);
  Client.Install (Node30);
  Client.Install (Node40);
  Client.Install (Node50);
  Client.Install (Node60);
  Client.Install (Node70);
  Client.Install (Node80);
  Client.Install (Node90);
  Client.Install (Node0);
 
  Client.Install (Node13);
  Client.Install (Node23);
  Client.Install (Node33);
  Client.Install (Node43);
  Client.Install (Node53);
  Client.Install (Node63);
  Client.Install (Node73);
  Client.Install (Node83);
  Client.Install (Node93);
  Client.Install (Node3);

  Client.Install (Node16);
  Client.Install (Node26);
  Client.Install (Node36);
  Client.Install (Node46);
  Client.Install (Node56);
  Client.Install (Node66);
  Client.Install (Node76);
  Client.Install (Node86);
  Client.Install (Node96);
  Client.Install (Node6);
 
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
  consumerHelper.Install (Node48);
 // consumerHelper.Install (Node30);
 // consumerHelper.Install (Node60);
  ccnxGlobalRoutingHelper.CalculateRoutes ();
  //app1.Start(Seconds (0.0)); // will send out Interest, which nobody will receive (Interests generated by an app will not got back to the app)
  //app1.Stop(Seconds(20.0));
  //app6.Start (Seconds (1.0)); // will send out an Interests, which will be received and satisfied by app1
  //app7.Start (Seconds (1.0));
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > >
  AggregateTracer = ndn::L3AggregateTracer::InstallAll ("data-bestroute/nc100/aggregate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3RateTracer> > >
  L3RateTracer = ndn::L3RateTracer::InstallAll ("data-bestroute/nc100/rate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::CsTracer> > >
  aggTracers = ndn::CsTracer::InstallAll ("data-bestroute/nc100/cs-trace.txt", Seconds (1));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::AppDelayTracer> > >
  delayTracer = ndn::AppDelayTracer::InstallAll ("data-bestroute/nc100/app-delays-trace.txt");

  Simulator::Stop (Seconds (600.0));
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
//}
//}
