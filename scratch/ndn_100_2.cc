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
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("1ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("100"));
  CommandLine cmd;
  cmd.Parse (argc, argv);
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("src/ndnSIM/examples/topologies/topo_tree.txt");
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
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
  //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
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
  Ptr<Node> Node100 = Names::Find<Node>("99");

  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node10);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node20);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node30);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node40);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node50);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node60);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node70);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node80);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node90);
  ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/nc", Node100);
  //ccnxGlobalRoutingHelper.CalculateRoutes ();

  ndn::AppHelper Client("ns3::ndn::ConsumerCbr");
  //ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");
  Client.SetPrefix("/ndn/vod/nc");
  Client.SetAttribute("Frequency", StringValue("10"));
  Client.Install (Node10);
  Client.Install (Node20);
  Client.Install (Node30);
  Client.Install (Node40);
  Client.Install (Node60);
  Client.Install (Node70);
  Client.Install (Node80);
  Client.Install (Node90);
  Client.Install (Node100);
  
  ndn::AppHelper producer("ns3::ndn::Producer");
  producer.SetPrefix("/ndn/vod/nc");
  producer.SetAttribute("PayloadSize", StringValue("1024"));
  producer.Install(Node50);
  //ccnxGlobalRoutingHelper.CalculateRoutes ();
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  //ApplicationContainer app4 = producer.Install(node4);
  
  //ndn::AppHelper consumerHelper ("NetworkCodingApp");
  //ApplicationContainer app1 = consumerHelper.Install (node1); 
  //ApplicationContainer app2 = consumerHelper.Install (node2);
  //ApplicationContainer app3 = consumerHelper.Install (node3);
  //ApplicationContainer app1 = producer.Install (node1);
  //consumerHelper.Install (Node50);
  ccnxGlobalRoutingHelper.CalculateRoutes ();
  //app1.Start(Seconds (0.0)); // will send out Interest, which nobody will receive (Interests generated by an app will not got back to the app)
  //app1.Stop(Seconds(20.0));
  //app6.Start (Seconds (1.0)); // will send out an Interests, which will be received and satisfied by app1
  //app7.Start (Seconds (1.0));
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > >
  AggregateTracer = ndn::L3AggregateTracer::InstallAll ("ndn100/aggregate-trace-BR.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3RateTracer> > >
  L3RateTracer = ndn::L3RateTracer::InstallAll ("ndn100/rate-trace-BR.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::CsTracer> > >
  aggTracers = ndn::CsTracer::InstallAll ("ndn100/cs-trace-BR.txt", Seconds (1));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::AppDelayTracer> > >
  delayTracer = ndn::AppDelayTracer::InstallAll ("ndn100/app-delays-trace-BR.txt");

  Simulator::Stop (Seconds (20.0));
  Simulator::Run ();
  /*for (NodeList::Iterator node = NodeList::Begin (); node != NodeList::End (); node ++)
  {
    std::cout << "Node #" << (*node)->GetId () << std::endl;
    (*node)->GetObject<ndn::ContentStore> ()->Print (std::cout);
      std::cout << std::endl;
  }*/
  Simulator::Destroy ();
  return 0;
}
