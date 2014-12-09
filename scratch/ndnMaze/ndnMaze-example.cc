/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/ndnMaze-helper.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/ndnMaze-module.h"
#include "ns3/annotated-topology-reader.h"
#include "ns3/ndn-app-delay-tracer.h"
#include "ns3/ndn-l3-aggregate-tracer.h"
#include "ns3/ndn-l3-rate-tracer.h"
#include "ns3/ndn-cs-tracer.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ndnMazePrototype");

string case_path;
const int uploader_num = 10;
const int scenario = 1;
const string dir = "ndnmaze_log";
int case_id;
int patch_num;
extern int downloader_num;

void measure_rtt_workload(int *a, int total_num, int active_num){
    ndnMazeLogger::SetSimulationId(1);
    ndnMazeLogger::SetLogDir(dir);
    ndnMazeDownloadTask::PARTNUM = patch_num;
    size_t b = case_path.find("case_sets_") + 10, e = case_path.find(".txt");
    string logSuffix = case_path.substr(b, e-b); 
    cout << "log suffix " << logSuffix << endl;
    ndnMazeLogger::SetRTTSuffix(logSuffix);
    ndnMazeLogger::SetDownloadLogSuffix(logSuffix);
    cout << " total node number: " << total_num << " active node num: " << active_num <<endl;
    
    cout << "reading topology ..." << endl;
    ns3::AnnotatedTopologyReader topologyReader ("", 25);
    string topoFile = "graduation_project/ns3topo100.txt";
    if(topoFile.find("200") != topoFile.npos){
        for(int i = 0; i < active_num; ++i) a[i] += 100;
    }
    topologyReader.SetFileName (topoFile);
    topologyReader.Read ();
    NodeContainer allNodes = topologyReader.GetNodes();
    cout << "topology reading finished" << endl;

    // Install CCNx stack on all nodes
    ndn::StackHelper ccnxHelper;
    ccnxHelper.SetPit ("ns3::ndn::pit::Lru","MaxSize", "0");
    ccnxHelper.SetContentStore ("ns3::ndn::cs::Lru","MaxSize", "0");
    //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::Flooding");
    ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::BestRoute");
    //ccnxHelper.SetForwardingStrategy ("ns3::ndn::fw::SmartFlooding");
    ccnxHelper.InstallAll ();

    // Installing global routing interface on all nodes
    ndn::GlobalRoutingHelper ccnxGlobalRoutingHelper;
    ccnxGlobalRoutingHelper.InstallAll ();
    
    int server_id;
    downloader_num = active_num - 1 - uploader_num;
    set<int> downloader_id, source_id, full_set;
    server_id = a[0]; full_set.insert(server_id); cout << server_id << " ";
    for(int j = 0; j < downloader_num; ++j){ cout << a[1+j] <<" "; downloader_id.insert(a[1+j]); full_set.insert(a[1+j]);}
    for(int j = 0; j < uploader_num; ++j) 
    { 
        source_id.insert(a[1+downloader_num+j]); full_set.insert(a[1+downloader_num+j]); cout << a[1+downloader_num+j] << " ";
    }
    //find nodes of different roles
    //find server node
    cout << "finding server node ..." << endl;
    stringstream ss;
    string name;
    ss << server_id; ss >> name;
    Ptr<Node> serverNode = Names::Find<Node>(name);
    if(!serverNode){ cout << "didn't find server node!!!" << endl; exit(1);}
    //find downloader nodes
    cout << "finding downloader nodes ..." << endl;
    NodeContainer downloaders;
    for(set<int>::iterator it = downloader_id.begin(); it != downloader_id.end(); ++it)
    {
        ss.clear(); ss << *it ; ss >> name; 
        Ptr<Node> downloader = Names::Find<Node>(name);
        downloaders.Add(downloader);
    }
    //find source nodes
    cout << "finding source nodes ..." << endl;
    NodeContainer sourceNodes;
    for(set<int>::iterator it = source_id.begin(); it != source_id.end(); ++it)
    {
        ss.clear(); ss << *it; ss >> name;
        Ptr<Node> source = Names::Find<Node>(name);
        sourceNodes.Add(source);
        //boost::shared_ptr<std::ofstream> outputStream (new std::ofstream ());
        //outputStream->open ((dir+"/"+name+"-cs-trace.txt").c_str(), std::ios_base::out | std::ios_base::trunc);
        //ndn::CsTracer aggTracers(outputStream, name); 
    }
    
    cout << "finding inactive nodes ..." << endl;
    NodeContainer otherNodes;
    for(int id = 0; id < total_num; ++id)
        if(full_set.find(id) == full_set.end())
        {
            ss.clear(); ss << id; ss >> name;
            Ptr<Node> node = Names::Find<Node>(name);
            otherNodes.Add(node);
        }

    // Install CCNx applications
    std::string prefix = "/netlab/ndnMaze";
    std::string dataPrefix = prefix + "/requestFile";
    cout << "installing server application ..." << endl;
    ndn::AppHelper serverHelper ("ns3::ndnMazeServer");
    //serverHelper.SetPrefix (prefix);
    serverHelper.SetAttribute ("ServerName", StringValue ("Maze Server")); 
    ApplicationContainer apps = serverHelper.Install (serverNode);
    for(NodeContainer::Iterator it = downloaders.Begin(); it != downloaders.End(); ++it){
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "downloader " + name; 
        ndn::AppHelper clientHelper1 ("ns3::ndnMazeClient");
        clientHelper1.SetAttribute ("DataPrefix", StringValue(dataPrefix));
        clientHelper1.SetAttribute ("ClientName", StringValue(name));
        clientHelper1.SetAttribute("SharedFilesScenario", UintegerValue(scenario-1));
        apps.Add(clientHelper1.Install (*it));
    }
    for(NodeContainer::Iterator it = sourceNodes.Begin(); it != sourceNodes.End(); ++it){
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "source " + name; 
        ndn::AppHelper clientHelper2 ("ns3::ndnMazeClient");
        clientHelper2.SetAttribute ("DataPrefix", StringValue(dataPrefix));
        clientHelper2.SetAttribute ("ClientName", StringValue(name));
        clientHelper2.SetAttribute("SharedFilesScenario", UintegerValue(scenario));
        apps.Add(clientHelper2.Install (*it));
    } 
    for(NodeContainer::Iterator it = otherNodes.Begin(); it != otherNodes.End(); ++it){
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "inactive node " + name; 
        ndn::AppHelper clientHelper2 ("ns3::ndnMazeClient");
        //clientHelper2.SetPrefix (prefix);
        clientHelper2.SetAttribute ("ClientName", StringValue(name));
        clientHelper2.SetAttribute("SharedFilesScenario", UintegerValue(7));
        apps.Add(clientHelper2.Install (*it));
    }

    // Add /prefix origins to ndn::GlobalRouter
    ccnxGlobalRoutingHelper.AddOrigins (prefix+"/server/login", serverNode);
    ccnxGlobalRoutingHelper.AddOrigins (dataPrefix, sourceNodes);

    // Calculate and install FIBs
    cout << "calculating routes ..." << endl;
    ccnxGlobalRoutingHelper.CalculateRoutes ();

    apps.Start(Seconds(0));
    //Simulator::Stop(Seconds(800)); 
    string trace_file;
    ss.clear(); ss << dir << "/" << logSuffix << "_" << case_id; ss >> ndnMazeClient::trace_prefix;
    trace_file = ndnMazeClient::trace_prefix + ".tr";
    //boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::L3AggregateTracer> > > aggTracers = ndn::L3AggregateTracer::InstallAll (trace_file, Seconds (6.0));
    string cs_trace = ndnMazeClient::trace_prefix + ".cstrace";
    boost::tuple< boost::shared_ptr<std::ostream>, std::list<Ptr<ndn::CsTracer> > >
    csTracers = ndn::CsTracer::InstallAll (cs_trace, Seconds (6));

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run ();
    NS_LOG_INFO("Done.");
    Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
    LogComponentEnable("ndnMazeClient", LOG_LEVEL_INFO);
    //LogComponentEnable("ndnMazeServer", LOG_LEVEL_DEBUG);
    LogComponentEnable("ndnMazePrototype", LOG_LEVEL_DEBUG);
    LogComponentEnable("ndnMazeDownloadTask", LOG_LEVEL_DEBUG);
    
    int sim_type, total_num, active_num;
    string node_list;
    
    CommandLine cmd;
    cmd.AddValue("case_id", "the id of current simulation", case_id);
    cmd.AddValue("case_path", "the path of the case file", case_path);
    cmd.AddValue("sim_type", "the type of simulation", sim_type);
    cmd.AddValue("total_num", "the total number of nodes", total_num);
    cmd.AddValue("active_num", "the number of active nodes", active_num);
    cmd.AddValue("node_list", "the active node list", node_list);
    cmd.AddValue("patch_num", "the number of interests sent as a patch at one time", patch_num);
    cmd.AddValue("min_patch", "the minimal patch size used to send interests", ndnMazeDownloadTask::MINPATCH);
    cmd.AddValue("max_rtt", "the maximum RTT value used for interest rate controal", ndnMazeClient::MAXRTT);
    //cmd.AddValue("inc_step", "the incremental step for interest rate control", ndnMazeClient::incStep);
    //cmd.AddValue("dec_step", "the decremental step for interest rate control", ndnMazeClient::decStep);
    cmd.Parse (argc, argv); 
   /* case_id = 1;
    case_path = "./graduation_project/case_sets_10.txt";
    sim_type = 1;
    total_num = 100;
    active_num = 21;
    node_list = "34,14,56,27,18,38,94,28,9,84,98,85,90,68,44,41,32,21,80,50,53";
    ndnMazeClient::MAXRTT = 1000;*/
    int *a = new int[active_num]; char comma;
    stringstream ss(node_list.c_str());
    int i;
    for(i = 0; i < active_num - 1; ++i)
    {
        ss >> a[i] >> comma; cout << a[i] << " ";
    }
    ss >> a[i]; cout << a[i] << endl;
    switch(sim_type)
    {
        case 1: measure_rtt_workload(a, total_num, active_num); break;
        case 2: ;break;
        case 3: ;break;
    }
    return 0;
}

