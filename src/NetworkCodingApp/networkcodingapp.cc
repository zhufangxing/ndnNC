#include "networkcodingapp.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "stdio.h"
#include "ns3/ndn-app-face.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-time-tag.h"
#include "ns3/string.h"

//#include "ns3/ndn-data.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndn-fib.h"
#include "ns3/random-variable.h"
#include "iostream"
#include "sstream"
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

using namespace std;

NS_LOG_COMPONENT_DEFINE ("NetworkCodingApp");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (NetworkCodingApp);
// register NS-3 type
TypeId NetworkCodingApp::GetTypeId ()
{
  static TypeId tid = TypeId ("NetworkCodingApp")
    .SetParent<ndn::App> ()
    .AddConstructor<NetworkCodingApp> ()
    .AddAttribute ("Prefix","Prefix, for which producer has the data",
                   StringValue ("/"),
                   MakeNameAccessor (&NetworkCodingApp::m_prefix),
                   MakeNameChecker ())
    .AddAttribute ("PayloadSize", "Virtual payload size for Content packets",
                   UintegerValue (1024),
                   MakeUintegerAccessor(&NetworkCodingApp::m_virtualPayloadSize),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&NetworkCodingApp::m_freshness),
                   MakeTimeChecker ())
    
    ;
  return tid;
}
// Processing upon start of the application
void NetworkCodingApp::StartApplication ()
{
  // initialize ndn::App
  ndn::App::StartApplication ();

  // Create a name components object for name ``/prefix/sub``
  //Ptr<ndn::Name> prefix = Create<ndn::Name> (); // now prefix contains ``/``
  //prefix->append ("ndn"); // now prefix contains ``/prefix``
  //prefix->append ("video"); // now prefix contains ``/prefix/sub``
  Ptr<ndn::NameComponents> prefix = Create<ndn::NameComponents> ("/ndn/vod/nc/");
  /////////////////////////////////////////////////////////////////////////////
  // Creating FIB entry that ensures that we will receive incoming Interests //
  /////////////////////////////////////////////////////////////////////////////

  // Get FIB object
  Ptr<ndn::Fib> fib = GetNode ()->GetObject<ndn::Fib> ();

  // Add entry to FIB
  // Note that ``m_face`` is cretaed by ndn::App
  Ptr<ndn::fib::Entry> fibEntry = fib->Add (*prefix, m_face, 0);
  fibEntry->UpdateStatus (m_face, ndn::fib::FaceMetric::NDN_FIB_GREEN);
  //Simulator::Schedule (Seconds (1.0), &NetworkCodingApp::SendInterest, this);
  NS_LOG_DEBUG("added one fib entry to ");
}
// Processing when application is stopped
void NetworkCodingApp::StopApplication ()
{
  // cleanup ndn::App
  ndn::App::StopApplication ();
}
void NetworkCodingApp::SendInterest ()
{
   /////////////////////////////////////
   // Sending one Interest packet out //
   /////////////////////////////////////
  
   /*Ptr<ndn::Name> prefix = Create<ndn::Name> ("/ndn/video/nc"); // another way to create name

   // Create and configure ndn::Interest
   Ptr<ndn::Interest> interest = Create<ndn::Interest> ();
   UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
   interest->SetNonce            (rand.GetValue ());
   interest->SetName             (prefix);
   interest->SetInterestLifetime (Seconds (10.0));

   NS_LOG_DEBUG ("Sending Interest packet for " << *prefix);
  
   // Call trace (for logging purposes)
   m_transmittedInterests (interest, this, m_face);*/
   Ptr<ndn::NameComponents> prefix = Create<ndn::NameComponents> ("/ndn/vod/nc/"); // another way to create name
   //Create and configure ndn::InterestHeader
   ndn::InterestHeader interestHeader;
   UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
   interestHeader.SetNonce (rand.GetValue ());
   interestHeader.SetName (prefix);
   interestHeader.SetInterestLifetime (Seconds (2.0));
   // Create packet and add ndn::InterestHeader
   Ptr<Packet> packet = Create<Packet> ();
   packet->AddHeader (interestHeader);
   NS_LOG_DEBUG ("[" << Simulator::Now() << "] "  << " Sending Interest for network coding data" << *prefix);
   // Forward packet to lower (network) layer
   m_protocolHandler (packet);
   // Call trace (for logging purposes)
   m_transmittedInterests (&interestHeader, this, m_face);
   //SgggcheduleNextPacket();
   //cout << " Send interest success "<< endl; 
   //m_face->ReceiveInterest (interest);
}

// Callback that will be called when Interest arrives
void NetworkCodingApp::OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket)
{
  uint32_t blocksNum=4;
  //uint32_t heap=1000;     //eg, 1, 1+heap, 1+2*heap, 1+3*heap  are coeded in a segment
  //uint32_t seqScop=blocksNum*heap;

   //std::cout<<"Producer-interest:"<<interest->GetName()<<" Coef:"<<interest->GetCoef()<<std::endl;
   App::OnInterest(interest,origPacket);
   NS_LOG_FUNCTION(this << interest);
   if(!m_active) return;
   const ndn::NameComponents name = interest->GetName().cut(1);
   ndn::NameComponents name2 = ndn::NameComponents("/ndn/vod/nc");
   //cout <<"Node :"<<GetNode()->GetId() <<" OnInterest: "<<interest->GetName() << " name: " << interest->GetName() <<"NC coef:"<<interest->GetCoef()<<endl; 
   if(name == name2)
   {
     NS_LOG_DEBUG ("Receiving Interest for Network Coding " << interest->GetName() );
     ndn::ContentObjectHeader data;
     list<string> prefix = interest->GetName().GetComponents();
     list<string>::iterator  iter;
     int num = 0;
     string vod, nc, seg, block = "";
     for (iter = prefix.begin(); iter != prefix.end(); iter++, num++)
     {
       if( num == 1)
          vod = *iter;
       if( num == 2)
          nc = *iter;
       if( num == 3)
          seg = *iter;
       if( num == 4)
          block = *iter;
     }
     //added by zfx
     //uint64_t coef=interest->GetCoef();
    // uint32_t seq_base=(boost::lexical_cast<int>(seg)/seqScop)*seqScop;
     //uint32_t seq_unit=boost::lexical_cast<int>(seg)%heap;
     //uint32_t seq_t;
     uint32_t data_coef;
//     int i=0;
     UniformVariable m_rand(0,blocksNum*2);
     while(true ){
        uint32_t j=m_rand.GetInteger(0,2*blocksNum-1);
	data_coef = j+10;
	//std::cout<<"j "<<j<<std::endl;
        if(!IsCoefSame(interest->GetCoef(),data_coef)) break;
        }
    // seg=boost::lexical_cast<string>(seq_t);
    // cout <<"媒体文件名称: " << vod << " 段号: "<< seg;
     //string randnum;
     //stringstream ss;
     //ss << random();
     //ss >> randnum;
     string dataname;
     //if( block != "")
     //   dataname = "/ndn/" + vod + "/" + nc + "/" + seg + "/" + block;
     //else
        dataname = "/ndn/" + vod + "/" + nc + "/" + seg +"/" + boost::lexical_cast<string>(data_coef);
     data.SetName (Create<ndn::NameComponents> (dataname));
     data.SetFreshness (m_freshness);
     //data.SetFreshness(Seconds (0.2));
     data.SetCoef(data_coef);
     data.SetSeq(boost::lexical_cast<int>(seg));
     data.SetTimestamp (Simulator::Now());
     data.SetSignature (0);
     static ndn::ContentObjectTail tailer; // doesn't require any configuration
     // Create packet and add header and trailer
     //unsigned char temp[1024] = "nc coef";
     //Ptr<Packet> packet = Create<Packet> (temp, 1024);
     Ptr<Packet> packet = Create<Packet> (1024);
     //packet->Serialize(temp, 1024);
     //packet->Print(cout);
     packet->AddHeader (data);
     packet->AddTrailer (tailer);
 
     ns3::ndn::FwHopCountTag hopCountTag;
     origPacket->RemovePacketTag(hopCountTag);
     ns3::ndn::FwHopCountTag hopCountTag2;     
     packet->AddPacketTag (hopCountTag2);  

     //add for INFORM forwarding
     ns3::ndn::FwHopTimeTag hopTimeTag;
     origPacket->RemovePacketTag(hopTimeTag);
     ns3::ndn::FwHopTimeTag hopTimeTag2;
     packet->AddPacketTag (hopTimeTag2);
     

     NS_LOG_DEBUG ("[" << Simulator::Now() << "] "<<" " << "Sending ContentObject packet for " << data.GetName ());
     // Call trace (for logging purposes)
     m_transmittedContentObjects (&data, packet, this, m_face);
     // Forward packet to lower (network) layer
     m_protocolHandler (packet);
     //cout <<" Send data: " << data.GetName() <<" coef: "<<data.GetCoef()<<"  success."<<"Freshness"<< data.GetFreshness()<< endl;
   }else
   {
     //cout << "unexpected Interest"<< interest->GetName()<<endl;
   }
}
// (overridden from ndn::App) Callback that will be called when Data arrives
void NetworkCodingApp::OnContentObject (const Ptr<const ndn::ContentObjectHeader> &contentObject,Ptr<Packet> payload)
{
    //NS_LOG_DEBUG ("Receiving Data packet for Network Coding " <<  contentObject->GetName() );
    ndn::App::OnContentObject(contentObject, payload);
    //cout <<"Node: "<<GetNode()->GetId()<<"DATA received for name " << contentObject->GetName() << endl;
}

//added by zfx
bool
 NetworkCodingApp::IsCoefSame(uint64_t coef_pit, uint64_t coef_data)
{
while(coef_pit>=10)
        {
        if(coef_pit%100==coef_data) return true;
        coef_pit/=100;
        }
return false;
}

} // namespace ndn
} // namespace ns3
