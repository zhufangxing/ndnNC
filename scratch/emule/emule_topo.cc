/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 */

// Network topology
// Default Network topology, 1 server (n0), 3 clients (n1-3)
/*
          n2    n3
           \   /
            \ /
             n0
             |
             |
             n1
*/
// - UDP multicast only
// - Tracing of queues and packet receptions to file "emule-prototye.tr"

#include <fstream>
#include "ns3/string.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/emule4ns3-module.h"
#include "ns3/csma-helper.h"
#include "ns3/annotated-topology-reader.h"
#include <string>
#include <sstream>
#include <set>
using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EmulePrototype");

string case_path; 
const int uploader_num = 1;
const int scenario = 1;
const string dir = "emule_log/";
int case_id;
extern int downloader_num;

Ipv4Address GetIpv4Address(Ptr<Node> node)
{
    Ptr<Ipv4L3Protocol> ipv4 = node->GetObject<Ipv4L3Protocol> ();
    if(!ipv4)
    {
        cout << "failed to get ipv4 protocol on " << node->GetId() << endl;
        return 0;
    }
    Ipv4Address addr = ipv4->GetAddress(1,0).GetLocal();
    cout << "node " << node->GetId() << " has IP address " << addr << endl;
    return addr; 
}

void measure_rtt_workload(int *a, int total_num, int active_num)
{
    EmuleLogger::SetSimulationId(1);
    EmuleLogger::SetLogDir(dir);
    size_t b = case_path.find("case_sets_") + 10, e = case_path.find(".txt");
    string logSuffix = case_path.substr(b, e-b); cout << "log suffix " << logSuffix << endl;
    EmuleLogger::SetRTTSuffix(logSuffix);
    EmuleLogger::SetDownloadLogSuffix(logSuffix);
    cout << " total node number: " << total_num << " active node num: " << active_num <<endl;
    cout << "reading topology ..." << endl;
    AnnotatedTopologyReader topologyReader ("", 25);
    topologyReader.SetFileName ("graduation_project/ns3topo100.txt");
    topologyReader.Read ();
    cout << "topology reading finished" << endl;

    InternetStackHelper internet;
    internet.InstallAll ();
    topologyReader.AssignIpv4Addresses("10.1.1.0");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

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
        ss.clear(); ss << *it ; ss >> name; cout << name << " ";
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
    cout << "installing server application ..." << endl;
    uint16_t udp_port = 1337;  // well-known echo port number
    std::string server_name = "eMuleServer 1";
    std::string server_desription = "First running eMule server of this prototype!";
    Ipv4Address serverAddress = GetIpv4Address(serverNode);
    cout << "server address " << serverAddress << endl;
    EmuleServerHelper server (udp_port,serverAddress, server_desription, server_name);
    ApplicationContainer apps = server.Install (serverNode);
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 1;
    Time interPacketInterval = MilliSeconds(300);
    cout << "installing downloader application ..." << endl;
    for(NodeContainer::Iterator it = downloaders.Begin(); it != downloaders.End(); ++it)
    {
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "downloader " + name; cout << name << endl;
        EmuleClientHelper c1 (serverAddress, udp_port, GetIpv4Address(*it), udp_port, name);
        c1.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
        c1.SetAttribute ("Interval", TimeValue (interPacketInterval));
        c1.SetAttribute ("PacketSize", UintegerValue (packetSize));
        c1.SetAttribute("SharedFilesScenario", UintegerValue (scenario - 1));
        apps.Add(c1.Install (*it));
    }
    cout << "installing source application ..." << endl;
    for(NodeContainer::Iterator it = sourceNodes.Begin(); it != sourceNodes.End(); ++it)
    {
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "source " + name; cout << name << endl;
        EmuleClientHelper c2 (serverAddress, udp_port,GetIpv4Address(*it), udp_port, name);
        c2.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
        c2.SetAttribute ("Interval", TimeValue (interPacketInterval));
        c2.SetAttribute ("PacketSize", UintegerValue (packetSize));
        c2.SetAttribute("SharedFilesScenario", UintegerValue (scenario));
        apps.Add(c2.Install (*it));
    }
    cout << "installing inactive application ..." << endl;
    for(NodeContainer::Iterator it = otherNodes.Begin(); it != otherNodes.End(); ++it)
    { 
        ss.clear(); ss << (*it)->GetId(); ss >> name; name = "inactive node " + name;
        EmuleClientHelper c2 (serverAddress, udp_port, GetIpv4Address(*it), udp_port, name);
        c2.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
        c2.SetAttribute ("Interval", TimeValue (interPacketInterval));
        c2.SetAttribute ("PacketSize", UintegerValue (packetSize));
        c2.SetAttribute("SharedFilesScenario", UintegerValue (7));
        apps.Add(c2.Install(*it));
    }

    string trace_file;
    ss.clear(); ss << dir << logSuffix << "_" << case_id << ".tr"; ss >> trace_file;
    AsciiTraceHelper ascii;
    internet.EnableAsciiIpv4All(ascii.CreateFileStream(trace_file));
    apps.Start (Seconds (0));
//    apps.Stop (Seconds (200.0));
    cout << "caculating routes ..." << endl;
    NS_LOG_INFO ("Run Simulation.");
    Simulator::Run ();
    NS_LOG_INFO ("Done.");
    Simulator::Destroy ();
}

int main(int argc, char *argv[])
{
    LogComponentEnable ("EmulePrototype", LOG_LEVEL_DEBUG);
    LogComponentEnable("EmuleClientApplication", LOG_LEVEL_ALL);
    LogComponentEnable("EmuleServerApplication", LOG_LEVEL_DEBUG);
    int sim_type, total_num, active_num;
    string node_list;
    CommandLine cmd;
    cmd.AddValue("case_id", "the id of current simulation", case_id);
    cmd.AddValue("case_path", "the path of the case file", case_path);
    cmd.AddValue("sim_type", "the type of simulation", sim_type);
    cmd.AddValue("total_num", "the total number of nodes", total_num);
    cmd.AddValue("active_num", "the number of active nodes", active_num);
    cmd.AddValue("node_list", "the active node list", node_list);
    cmd.Parse (argc, argv);
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

