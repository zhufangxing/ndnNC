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
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("100"));
  CommandLine cmd;
  cmd.Parse (argc, argv);
  AnnotatedTopologyReader topologyReader ("", 10);
  topologyReader.SetFileName ("topo/topo100-zfx.txt");
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
ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize","4000");   //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
// ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
  ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Multipath");
  ccnxHelper.InstallAll ();
  
  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
  ccnxGlobalRoutingHelper.InstallAll ();
  //Add /prefix origins to ndn::GlobalRouter
Ptr<Node> Node0 = Names::Find<Node>("0");
Ptr<Node> Node2 = Names::Find<Node>("2");
Ptr<Node> Node4 = Names::Find<Node>("4");
Ptr<Node> Node6 = Names::Find<Node>("6");
Ptr<Node> Node8 = Names::Find<Node>("8");
Ptr<Node> Node10 = Names::Find<Node>("10");
Ptr<Node> Node12 = Names::Find<Node>("12");
Ptr<Node> Node14 = Names::Find<Node>("14");
Ptr<Node> Node16 = Names::Find<Node>("16");
Ptr<Node> Node18 = Names::Find<Node>("18");
Ptr<Node> Node20 = Names::Find<Node>("20");
Ptr<Node> Node22 = Names::Find<Node>("22");
Ptr<Node> Node24 = Names::Find<Node>("24");
Ptr<Node> Node26 = Names::Find<Node>("26");
Ptr<Node> Node28 = Names::Find<Node>("28");
Ptr<Node> Node30 = Names::Find<Node>("30");
Ptr<Node> Node32 = Names::Find<Node>("32");
Ptr<Node> Node34 = Names::Find<Node>("34");
Ptr<Node> Node36 = Names::Find<Node>("36");
Ptr<Node> Node38 = Names::Find<Node>("38");
Ptr<Node> Node40 = Names::Find<Node>("40");
Ptr<Node> Node42 = Names::Find<Node>("42");
Ptr<Node> Node44 = Names::Find<Node>("44");
Ptr<Node> Node46 = Names::Find<Node>("46");
Ptr<Node> Node48 = Names::Find<Node>("48");
Ptr<Node> Node50 = Names::Find<Node>("50");
Ptr<Node> Node52 = Names::Find<Node>("52");
Ptr<Node> Node54 = Names::Find<Node>("54");
Ptr<Node> Node56 = Names::Find<Node>("56");
Ptr<Node> Node58 = Names::Find<Node>("58");
Ptr<Node> Node60 = Names::Find<Node>("60");
Ptr<Node> Node62 = Names::Find<Node>("62");
Ptr<Node> Node64 = Names::Find<Node>("64");
Ptr<Node> Node66 = Names::Find<Node>("66");
Ptr<Node> Node68 = Names::Find<Node>("68");
Ptr<Node> Node70 = Names::Find<Node>("70");
Ptr<Node> Node72 = Names::Find<Node>("72");
Ptr<Node> Node74 = Names::Find<Node>("74");
Ptr<Node> Node76 = Names::Find<Node>("76");
Ptr<Node> Node78 = Names::Find<Node>("78");
Ptr<Node> Node80 = Names::Find<Node>("80");
Ptr<Node> Node82 = Names::Find<Node>("82");
Ptr<Node> Node84 = Names::Find<Node>("84");
Ptr<Node> Node86 = Names::Find<Node>("86");
Ptr<Node> Node88 = Names::Find<Node>("88");
Ptr<Node> Node90 = Names::Find<Node>("90");
Ptr<Node> Node92 = Names::Find<Node>("92");
Ptr<Node> Node94 = Names::Find<Node>("94");
Ptr<Node> Node96 = Names::Find<Node>("96");
Ptr<Node> Node98 = Names::Find<Node>("98");
ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/ndn", Node30);
ccnxGlobalRoutingHelper.AddOrigins ("/ndn/vod/ndn", Node60);
ndn::AppHelper Client("ns3::ndn::ConsumerCbr");
//ndn::AppHelper Client("ns3::ndn::ConsumerZipfMandelbrot");
Client.SetPrefix("/ndn/vod/ndn");
Client.SetAttribute("Frequency", StringValue("200"));
Client.SetAttribute("Randomize", StringValue ("uniform"));
ApplicationContainer app0 = Client.Install (Node0);
ApplicationContainer app2 = Client.Install (Node2);
ApplicationContainer app4 = Client.Install (Node4);
ApplicationContainer app6 = Client.Install (Node6);
ApplicationContainer app8 = Client.Install (Node8);
ApplicationContainer app10 = Client.Install (Node10);
ApplicationContainer app12 = Client.Install (Node12);
ApplicationContainer app14 = Client.Install (Node14);
ApplicationContainer app16 = Client.Install (Node16);
ApplicationContainer app18 = Client.Install (Node18);
ApplicationContainer app20 = Client.Install (Node20);
ApplicationContainer app22 = Client.Install (Node22);
ApplicationContainer app24 = Client.Install (Node24);
ApplicationContainer app26 = Client.Install (Node26);
ApplicationContainer app28 = Client.Install (Node28);
ApplicationContainer app32 = Client.Install (Node32);
ApplicationContainer app34 = Client.Install (Node34);
ApplicationContainer app36 = Client.Install (Node36);
ApplicationContainer app38 = Client.Install (Node38);
ApplicationContainer app40 = Client.Install (Node40);
ApplicationContainer app42 = Client.Install (Node42);
ApplicationContainer app44 = Client.Install (Node44);
ApplicationContainer app46 = Client.Install (Node46);
ApplicationContainer app48 = Client.Install (Node48);
ApplicationContainer app50 = Client.Install (Node50);
ApplicationContainer app52 = Client.Install (Node52);
ApplicationContainer app54 = Client.Install (Node54);
ApplicationContainer app56 = Client.Install (Node56);
ApplicationContainer app58 = Client.Install (Node58);
ApplicationContainer app62 = Client.Install (Node62);
ApplicationContainer app64 = Client.Install (Node64);
ApplicationContainer app66 = Client.Install (Node66);
ApplicationContainer app68 = Client.Install (Node68);
ApplicationContainer app70 = Client.Install (Node70);
ApplicationContainer app72 = Client.Install (Node72);
ApplicationContainer app74 = Client.Install (Node74);
ApplicationContainer app76 = Client.Install (Node76);
ApplicationContainer app78 = Client.Install (Node78);
ApplicationContainer app80 = Client.Install (Node80);
ApplicationContainer app82 = Client.Install (Node82);
ApplicationContainer app84 = Client.Install (Node84);
ApplicationContainer app86 = Client.Install (Node86);
ApplicationContainer app88 = Client.Install (Node88);
ApplicationContainer app90 = Client.Install (Node90);
ApplicationContainer app92 = Client.Install (Node92);
ApplicationContainer app94 = Client.Install (Node94);
ApplicationContainer app96 = Client.Install (Node96);
ApplicationContainer app98 = Client.Install (Node98);
 
  ndn::AppHelper producer("ns3::ndn::Producer");
  producer.SetPrefix("/ndn/vod/ndn");
  producer.SetAttribute("PayloadSize", StringValue("1024"));
  producer.Install(Node30);
  producer.Install(Node60);
  //ccnxGlobalRoutingHelper.CalculateRoutes ();
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
  std::string path="data-bestroute/nc100";
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > >
  AggregateTracer = ndn::L3AggregateTracer::InstallAll (path+"/aggregate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3RateTracer> > >
  L3RateTracer = ndn::L3RateTracer::InstallAll (path+"/rate-trace.txt", Seconds (0.5));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::CsTracer> > >
  aggTracers = ndn::CsTracer::InstallAll (path+"/cs-trace.txt", Seconds (1));
 
  boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::AppDelayTracer> > >
  delayTracer = ndn::AppDelayTracer::InstallAll (path+"/app-delays-trace.txt");

Simulator::Stop (Seconds (60)); 
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
