/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:  Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 *          Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "ndn-forwarding-strategy.h"

#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-fib.h"
#include "ns3/ndn-content-store.h"
#include "ns3/ndn-face.h"
#include "ns3/ndn-app-face.h"
#include "ns3/assert.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/boolean.h"
#include "ns3/string.h"

#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
 #include "ns3/ndnSIM/utils/ndn-fw-hop-time-tag.h"

#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include "iostream"
#include "stdio.h"
using namespace std;

namespace ll = boost::lambda;

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ForwardingStrategy);

NS_LOG_COMPONENT_DEFINE (ForwardingStrategy::GetLogName ().c_str ());

std::string
ForwardingStrategy::GetLogName ()
{
  return "ndn.fw";
}

TypeId ForwardingStrategy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ForwardingStrategy")
    .SetGroupName ("Ndn")
    .SetParent<Object> ()

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    .AddTraceSource ("OutInterests",  "OutInterests",  MakeTraceSourceAccessor (&ForwardingStrategy::m_outInterests))
    .AddTraceSource ("InInterests",   "InInterests",   MakeTraceSourceAccessor (&ForwardingStrategy::m_inInterests))
    .AddTraceSource ("DropInterests", "DropInterests", MakeTraceSourceAccessor (&ForwardingStrategy::m_dropInterests))

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    .AddTraceSource ("OutData",  "OutData",  MakeTraceSourceAccessor (&ForwardingStrategy::m_outData))
    .AddTraceSource ("InData",   "InData",   MakeTraceSourceAccessor (&ForwardingStrategy::m_inData))
    .AddTraceSource ("DropData", "DropData", MakeTraceSourceAccessor (&ForwardingStrategy::m_dropData))

    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////

    .AddTraceSource ("SatisfiedInterests",  "SatisfiedInterests",  MakeTraceSourceAccessor (&ForwardingStrategy::m_satisfiedInterests))
    .AddTraceSource ("TimedOutInterests",   "TimedOutInterests",   MakeTraceSourceAccessor (&ForwardingStrategy::m_timedOutInterests))

    .AddAttribute ("CacheUnsolicitedDataFromApps", "Cache unsolicited data that has been pushed from applications",
                   BooleanValue (true),
                   MakeBooleanAccessor (&ForwardingStrategy::m_cacheUnsolicitedDataFromApps),
                   MakeBooleanChecker ())
    
    .AddAttribute ("CacheUnsolicitedData", "Cache overheard data that have not been requested",
                   BooleanValue (false),
                   MakeBooleanAccessor (&ForwardingStrategy::m_cacheUnsolicitedData),
                   MakeBooleanChecker ())

    .AddAttribute ("DetectRetransmissions", "If non-duplicate interest is received on the same face more than once, "
                                            "it is considered a retransmission",
                   BooleanValue (true),
                   MakeBooleanAccessor (&ForwardingStrategy::m_detectRetransmissions),
                   MakeBooleanChecker ())
    ;
  return tid;
}

ForwardingStrategy::ForwardingStrategy ()
{
}

ForwardingStrategy::~ForwardingStrategy ()
{
}

void
ForwardingStrategy::NotifyNewAggregate ()
{
  if (m_pit == 0)
    {
      m_pit = GetObject<Pit> ();
    }
  if (m_fib == 0)
    {
      m_fib = GetObject<Fib> ();
    }
  if (m_contentStore == 0)
    {
      m_contentStore = GetObject<ContentStore> ();
    }

  Object::NotifyNewAggregate ();
}

void
ForwardingStrategy::DoDispose ()
{
  m_pit = 0;
  m_contentStore = 0;
  m_fib = 0;

  Object::DoDispose ();
}

void
ForwardingStrategy::OnInterest (Ptr<Face> inFace,
                                Ptr<Interest> header,
                                Ptr<const Packet> origPacket)
{
  m_inInterests (header, inFace);
  // modified by pengcheng
  //std::cout << "FW Lookup header:" << *header << "  Name:"<< header->GetName()<< std::endl;
  //string name, match;
  Name interest_name = header->GetName();
  //match = 
  list<string> prefix = header->GetName().GetComponents();
  list<string>::iterator  iter;
  int num = 0;
  string vod, nc, seg;
  uint32_t blocksNum=4;
  uint32_t heap=500;     //eg, 1, 1+heap, 1+2*heap, 1+3*heap  are coeded in a segment
  uint32_t seqScop=blocksNum*heap;

  for (iter = prefix.begin(); iter != prefix.end(); iter++, num++)
   {
    if( num == 1)
       vod = *iter;
    if( num == 2)
       nc = *iter;
    if( num == 3)
       seg = *iter;
  }
  //const::string matchname = "/ndn/"+vod+"/"+nc;
  //std::cout << " New Name: " << matchname<<std::endl;
  //ndn::NameComponents name = ndn::NameComponents(matchname);
  //if (nc != 0)
  //{
  //  Ptr<pit::Entry> pitEntry = m_pit->Lookup(*ncheader);
  //}
  //else
  //  Ptr<pit::Entry> pitEntry = m_pit->Lookup(*header);
  //end 
  Ptr<pit::Entry> pitEntry = m_pit->Lookup(*header);
  bool similarInterest = true;
  if (pitEntry == 0)
    {
      similarInterest = false;
      pitEntry = m_pit->Create (header);
      if (pitEntry != 0)
        {
          DidCreatePitEntry (inFace, header, origPacket, pitEntry);
        }
      else
        {
          FailedToCreatePitEntry (inFace, header, origPacket);
          return;
        }
    }

  bool isDuplicated = true;
  if (!pitEntry->IsNonceSeen (header->GetNonce ()))
    {
      pitEntry->AddSeenNonce (header->GetNonce ());
      isDuplicated = false;
    }

  if (isDuplicated)
    {
      DidReceiveDuplicateInterest (inFace, header, origPacket, pitEntry);
      return;
    }

  Ptr<Packet> contentObject;
  Ptr<const ContentObject> contentObjectHeader; // used for tracing
  Ptr<const Packet> payload; // used for tracing
  //std::cout <<"FW: Lookup " << header->GetName()<<" coef: "<<header->GetCoef()<<" Nonce: "<<header->GetNonce()<<std::endl;

  //modified by zfx
  uint32_t seq=boost::lexical_cast<uint32_t>( seg);
  uint32_t seq_base=(seq/seqScop)*seqScop;
  uint32_t seq_unit=seq%heap;
  uint64_t coef_int=header->GetCoef();
  if(nc=="nc")
  for(uint32_t i=0;i<blocksNum;i++){
  for(uint32_t j=10;j<=blocksNum*2+9;j++){
  Ptr<Name> nameWithSequence = Create<Name> ("ndn/vod/nc");
  uint32_t  seq_t=i*heap+seq_base+seq_unit;
//  if(IsCoefSame(coef_int,seq_t%seqScop+10)) continue;
//  (*nameWithSequence) ("ndn");
//  (*nameWithSequence) (vod);
//  (*nameWithSequence) (nc);
  (*nameWithSequence) (seq_t);
  (*nameWithSequence) (j);
 
// Interest header_nc;
//  header_nc.SetNonce               (header->GetNonce());
  header->SetName                (nameWithSequence);
//  header_nc.SetInterestLifetime    (header->GetInterestLifetime());
//  header_nc.SetScope               (header->GetScope());
//  header_nc.SetCoef                (header->GetCoef());
//  header_nc.SetNack                (header->GetNack());
  //interestHeader.SetCoef        
  //Ptr<Interest> header_nc=&interestHeader;
  //boost::tie (contentObject, contentObjectHeader, payload) = m_contentStore->Lookup (&header_nc);
//std::cout <<"header_nc name"<<&header_nc.GetName()<<std::endl;
//std::cout <<"header name"<<&header->GetName()<<std::endl;

  //cs trace in Lookup, added by zfx
  boost::tie (contentObject, contentObjectHeader, payload) = m_contentStore->Lookup_nc (header, i==(blocksNum-1) && j==(9+blocksNum*2), coef_int);
  if (contentObject != 0 && !IsCoefSame(coef_int,contentObjectHeader->GetCoef()))
    {
      NS_ASSERT (contentObjectHeader != 0);

      FwHopCountTag hopCountTag;
      // if (origPacket->PeekPacketTag (hopCountTag))
      //   {
          contentObject->AddPacketTag (hopCountTag);
        // }

      // add for PINFOM forarding, 2015.12.24
      FwHopTimeTag hopTimetag;
      if(origPacket->PeekPacketTag (hopTimetag))
        {
          hopTimetag.Update(pitEntry->GetFibEntry()->m_QExpectInExplo);
          contentObject->AddPacketTag (hopTimetag);
        }

      pitEntry->AddIncoming (inFace, header->GetCoef());
      // Do data plane performance measurements
     // WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyIncomingInterest (inFace, contentObjectHeader, payload, contentObject, pitEntry);
//std::cout<<"FW-interest:"<<header->GetName()<<"Hit "<< "interest-coef"<<coef_int<<std::endl;
      return;
    }
	
  } 
  } 

else{//without network coding
  boost::tie (contentObject, contentObjectHeader, payload) = m_contentStore->Lookup (header);
  if (contentObject != 0)
    {
      NS_ASSERT (contentObjectHeader != 0);

      FwHopCountTag hopCountTag;
      // if (origPacket->PeekPacketTag (hopCountTag))
      //   {
          contentObject->AddPacketTag (hopCountTag);
        // }

        // add for PINFOM forarding, 2015.12.24
      FwHopTimeTag hopTimetag;
      if(origPacket->PeekPacketTag (hopTimetag))
        {
          hopTimetag.Update(pitEntry->GetFibEntry()->m_QExpectInExplo);
          contentObject->AddPacketTag (hopTimetag);
        }

      pitEntry->AddIncoming (inFace, header->GetCoef());

      // Do data plane performance measurements
      WillSatisfyPendingInterest (0, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (0, contentObjectHeader, payload, contentObject, pitEntry);
      return;
    }
}
	//added by zfx
	header->SetName(interest_name);
  if (similarInterest && ShouldSuppressIncomingInterest (inFace, header, origPacket, pitEntry))
    {
      pitEntry->AddIncoming (inFace, header->GetCoef());
      // update PIT entry lifetime
      pitEntry->UpdateLifetime (header->GetInterestLifetime ());

      // Suppress this interest if we're still expecting data from some other face
      NS_LOG_DEBUG ("Suppress interests");
      m_dropInterests (header, inFace);

      DidSuppressSimilarInterest (inFace, header, origPacket, pitEntry);
      return;
    }

  if (similarInterest)
    {
      DidForwardSimilarInterest (inFace, header, origPacket, pitEntry);
    }
//std::cout<<"FW-interest:"<<header->GetName()<<std::endl;
  PropagateInterest (inFace, header, origPacket, pitEntry);
  header->SetPathNum(1);
}

void
ForwardingStrategy::OnData (Ptr<Face> inFace,
                            Ptr<const ContentObject> header,
                            Ptr<Packet> payload,
                            Ptr<const Packet> origPacket)
{
  NS_LOG_FUNCTION (inFace << header->GetName () << payload << origPacket);
  //Tracer InData
  m_inData (header, payload, inFace);

  //modified by zfx
  uint32_t blocksNum=4;
  uint32_t heap=500;     //eg, 1, 1+heap, 1+2*heap, 1+3*heap  are coeded in a segment
  uint32_t seqScop=blocksNum*heap;

  list<string> prefix = header->GetName().GetComponents();
  list<string>::iterator  iter;
  int num = 0;
  string vod, nc, seg;
  for (iter = prefix.begin(); iter != prefix.end(); iter++, num++)
   {
    if( num == 1)
       vod = *iter;
    if( num == 2)
       nc = *iter;
    if( num == 3)
       seg = *iter;
  }


if(nc=="nc"){
//    uint32_t seq_0=boost::lexical_cast<uint32_t>(seg);
    uint32_t seq_0 = header->GetSeq();
    uint64_t coef_data=header->GetCoef();
//std::cout<<"FW-OnData:"<<header->GetName()<<" coef :"<<coef_data<<std::endl;
    uint32_t seq_base=(seq_0/seqScop)*seqScop;
    uint32_t seq_unit=seq_0%heap;
    Ptr<pit::Entry> pitEntry;
    Ptr<ContentObject> header_t;
  bool isPitHit = false;
  for(uint32_t i=0;i<blocksNum;i++){
  uint32_t seq=seq_base+seq_unit+i*heap;
  Ptr<Name> nameWithSequence = Create<Name> ("ndn/vod/nc");
//  (*nameWithSequence) ("ndn");
//  (*nameWithSequence) (vod);
//  (*nameWithSequence) (nc);
  (*nameWithSequence) (seq);
 
 Interest interestHeader;
  //interestHeader.SetNonce               (m_rand.GetValue ());
  interestHeader.SetName                (nameWithSequence);
  //interestHeader.SetInterestLifetime    (m_interestLifeTime);
  //interestHeader.SetCoef        
  header_t = Create<ContentObject> ();
  header_t->SetName (Create<Name> (interestHeader.GetName()));
  pitEntry = m_pit->Lookup (*header_t);
  //uint64_t coef_pit = pitEntry->GetCoef();

	if(i==0){
          FwHopCountTag hopCountTag;

          Ptr<Packet> payloadCopy = payload->Copy ();
          payloadCopy->RemovePacketTag (hopCountTag);

          // Optimistically add or update entry in the content store
          m_contentStore->Add (header, payloadCopy);
 	}
 while (pitEntry != 0)
    {
     // if(!IsCoefSame(pitEntry->GetIncoming().m_face,coef_data)){
	isPitHit=true;
      // Do data plane performance measurements
      WillSatisfyPendingInterest (inFace, pitEntry);

      // Actually satisfy pending interest
     // SatisfyPendingInterest (inFace, header_t, payload, origPacket, pitEntry);
      SatisfyPendingInterest_nc (inFace, header_t, payload, origPacket, pitEntry, coef_data);
//	}
      // Lookup another PIT entry
      pitEntry = m_pit->Lookup (*header_t);
    }
}
    if(!isPitHit) 
        {
          m_dropData (header, payload, inFace);
          NS_LOG_DEBUG ("Cannot satisfy data to " << inFace);
        }
 
// if (pitEntry == 0)
 // if(!isPitHit) ; 
//  {
//      bool cached = false;
//
//      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () | Face::APPLICATION)))
//        {
//          FwHopCountTag hopCountTag;
//
//          Ptr<Packet> payloadCopy = payload->Copy ();
//          payloadCopy->RemovePacketTag (hopCountTag);
//
//          // Optimistically add or update entry in the content store
//          cached = m_contentStore->Add (header, payloadCopy);
//        }
//      else
//        {
//          // Drop data packet if PIT entry is not found
//          // (unsolicited data packets should not "poison" content store)
//
//          //drop dulicated or not requested data packet
//          m_dropData (header, payload, inFace);
//        }
//
//      DidReceiveUnsolicitedData (inFace, header, payload, origPacket, cached);
//      return;
//    }
//  else
//    {
//      bool cached = false;
//
//      FwHopCountTag hopCountTag;
//      if (payload->PeekPacketTag (hopCountTag))
//        {
//          Ptr<Packet> payloadCopy = payload->Copy ();
//          payloadCopy->RemovePacketTag (hopCountTag);
//
//          // Add or update entry in the content store
//          cached = m_contentStore->Add (header, payloadCopy);
//        }
//      else
//        {
//          // Add or update entry in the content store
//          cached = m_contentStore->Add (header, payload); // no need for extra copy
//        }
//
//      DidReceiveSolicitedData (inFace, header, payload, origPacket, cached);
//    }

} 

//ndn without networkcoding 
else{
 Ptr<pit::Entry> pitEntry = m_pit->Lookup (*header);
  if (pitEntry == 0)
    {
      bool cached = false;

      if (m_cacheUnsolicitedData || (m_cacheUnsolicitedDataFromApps && (inFace->GetFlags () | Face::APPLICATION)))
        {
          FwHopCountTag hopCountTag;

          Ptr<Packet> payloadCopy = payload->Copy ();
          payloadCopy->RemovePacketTag (hopCountTag);

          // Optimistically add or update entry in the content store
          cached = m_contentStore->Add (header, payloadCopy);
        }
      else
        {
          // Drop data packet if PIT entry is not found
          // (unsolicited data packets should not "poison" content store)

          //drop dulicated or not requested data packet
          m_dropData (header, payload, inFace);
        }

      DidReceiveUnsolicitedData (inFace, header, payload, origPacket, cached);
      return;
    }
  else
    {
      bool cached = false;

      FwHopCountTag hopCountTag;
      if (payload->PeekPacketTag (hopCountTag))
        {
          Ptr<Packet> payloadCopy = payload->Copy ();
          payloadCopy->RemovePacketTag (hopCountTag);

          // Add or update entry in the content store
          cached = m_contentStore->Add (header, payloadCopy);
        }
      else
        {
          // Add or update entry in the content store
          cached = m_contentStore->Add (header, payload); // no need for extra copy
        }

      DidReceiveSolicitedData (inFace, header, payload, origPacket, cached);
    }

  while (pitEntry != 0)
    {
      // Do data plane performance measurements
      WillSatisfyPendingInterest (inFace, pitEntry);

      // Actually satisfy pending interest
      SatisfyPendingInterest (inFace, header, payload, origPacket, pitEntry);

      // Lookup another PIT entry
      pitEntry = m_pit->Lookup (*header);
    }
}
}
//added by zfx
bool
ForwardingStrategy::IsCoefSame(uint64_t coef_pit, uint64_t coef_data)
{
//std::cout<<"IsCoefSame:"<<coef_pit<<" "<<coef_data<<std::endl;
while(coef_pit>=10)
	{
	if(coef_pit%100==coef_data) return true;
	coef_pit/=100;
	}
return false;
}
void
ForwardingStrategy::DidCreatePitEntry (Ptr<Face> inFace,
                                       Ptr<const Interest> header,
                                       Ptr<const Packet> origPacket,
                                       Ptr<pit::Entry> pitEntrypitEntry)
{
}

void
ForwardingStrategy::FailedToCreatePitEntry (Ptr<Face> inFace,
                                            Ptr<const Interest> header,
                                            Ptr<const Packet> origPacket)
{
  m_dropInterests (header, inFace);
}

void
ForwardingStrategy::DidReceiveDuplicateInterest (Ptr<Face> inFace,
                                                 Ptr<const Interest> header,
                                                 Ptr<const Packet> origPacket,
                                                 Ptr<pit::Entry> pitEntry)
{
  /////////////////////////////////////////////////////////////////////////////////////////
  //                                                                                     //
  // !!!! IMPORTANT CHANGE !!!! Duplicate interests will create incoming face entry !!!! //
  //                                                                                     //
  /////////////////////////////////////////////////////////////////////////////////////////
  pitEntry->AddIncoming (inFace, header->GetCoef());
  m_dropInterests (header, inFace);
}

void
ForwardingStrategy::DidSuppressSimilarInterest (Ptr<Face> face,
                                                Ptr<const Interest> header,
                                                Ptr<const Packet> origPacket,
                                                Ptr<pit::Entry> pitEntry)
{
}

void
ForwardingStrategy::DidForwardSimilarInterest (Ptr<Face> inFace,
                                               Ptr<const Interest> header,
                                               Ptr<const Packet> origPacket,
                                               Ptr<pit::Entry> pitEntry)
{
}

void
ForwardingStrategy::DidExhaustForwardingOptions (Ptr<Face> inFace,
                                                 Ptr<const Interest> header,
                                                 Ptr<const Packet> origPacket,
                                                 Ptr<pit::Entry> pitEntry)
{
  NS_LOG_FUNCTION (this << boost::cref (*inFace));
  if (pitEntry->AreAllOutgoingInVain ())
    {
      m_dropInterests (header, inFace);

      // All incoming interests cannot be satisfied. Remove them
      pitEntry->ClearIncoming ();

      // Remove also outgoing
      pitEntry->ClearOutgoing ();

      // Set pruning timout on PIT entry (instead of deleting the record)
      m_pit->MarkErased (pitEntry);
    }
}



bool
ForwardingStrategy::DetectRetransmittedInterest (Ptr<Face> inFace,
                                                 Ptr<const Interest> header,
                                                 Ptr<const Packet> packet,
                                                 Ptr<pit::Entry> pitEntry)
{
  pit::Entry::in_iterator existingInFace = pitEntry->GetIncoming ().find (inFace);

  bool isRetransmitted = false;

  if (existingInFace != pitEntry->GetIncoming ().end ())
    {
      // this is almost definitely a retransmission. But should we trust the user on that?
      isRetransmitted = true;
    }

  return isRetransmitted;
}

void
ForwardingStrategy::SatisfyPendingInterest (Ptr<Face> inFace,
                                            Ptr<const ContentObject> header,
                                            Ptr<const Packet> payload,
                                            Ptr<const Packet> origPacket,
                                            Ptr<pit::Entry> pitEntry)
{
  if (inFace != 0)
    pitEntry->RemoveIncoming (inFace);

  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
      bool ok = incoming.m_face->Send (origPacket->Copy ());
      DidSendOutData (inFace, incoming.m_face, header, payload, origPacket, pitEntry);
      NS_LOG_DEBUG ("Satisfy " << *incoming.m_face);

      if (!ok)
        {
          m_dropData (header, payload, incoming.m_face);
          NS_LOG_DEBUG ("Cannot satisfy data to " << *incoming.m_face);
        }
    }
  // All incoming interests are satisfied. Remove them
  pitEntry->ClearIncoming ();

  // Remove all outgoing faces
  pitEntry->ClearOutgoing ();

  // Set pruning timout on PIT entry (instead of deleting the record)
  m_pit->MarkErased (pitEntry);
}

void
ForwardingStrategy::SatisfyIncomingInterest (Ptr<Face> inFace,
                                            Ptr<const ContentObject> header,
                                            Ptr<const Packet> payload,
                                            Ptr<const Packet> origPacket,
                                            Ptr<pit::Entry> pitEntry)
{

  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
	if(incoming.m_face!=inFace) continue;
      bool ok = incoming.m_face->Send (origPacket->Copy ());
      DidSendOutData (inFace, incoming.m_face, header, payload, origPacket, pitEntry);
      NS_LOG_DEBUG ("Satisfy " << *incoming.m_face);

      if (!ok)
        {
          m_dropData (header, payload, incoming.m_face);
          NS_LOG_DEBUG ("Cannot satisfy data to " << *incoming.m_face);
        }
    }

   		pitEntry->RemoveIncoming (inFace);
  // Remove all outgoing faces
 // pitEntry->ClearOutgoing ();

  // Set pruning timout on PIT entry (instead of deleting the record)
 // m_pit->MarkErased (pitEntry);
}

void
ForwardingStrategy::SatisfyPendingInterest_nc (Ptr<Face> inFace,
                                            Ptr<const ContentObject> header,
                                            Ptr<const Packet> payload,
                                            Ptr<const Packet> origPacket,
                                            Ptr<pit::Entry> pitEntry,
					    uint64_t coef_data)
{
  //satisfy all pending incoming Interests
  BOOST_FOREACH (const pit::IncomingFace &incoming, pitEntry->GetIncoming ())
    {
	if(IsCoefSame(incoming.m_coef,coef_data) ) continue;
	if(inFace== incoming.m_face) {
   		pitEntry->RemoveIncoming (inFace);
		continue;
		}
      bool ok = incoming.m_face->Send (origPacket->Copy ());
      DidSendOutData (inFace, incoming.m_face, header, payload, origPacket, pitEntry);
      NS_LOG_DEBUG ("Satisfy " << *incoming.m_face);

      if (!ok)
        {
          m_dropData (header, payload, incoming.m_face);
          NS_LOG_DEBUG ("Cannot satisfy data to " << *incoming.m_face);
        }
   		pitEntry->RemoveIncoming (inFace);
    }
  // Remove all outgoing faces
  pitEntry->ClearOutgoing ();

  // Set pruning timout on PIT entry (instead of deleting the record)
  m_pit->MarkErased (pitEntry);
//############ version 1 #########################
// std::set<uint64_t>::iterator it_coef = pitEntry->GetCoef().begin();
//  //satisfy all pending incoming Interests
//for (std::set<pit::IncomingFace >::iterator it_face =  pitEntry->GetIncoming ().begin()
//	; it_face !=  pitEntry->GetIncoming ().end() ; it_coef++,it_face++) 
//	{
//	if(it_coef==pitEntry->GetCoef().end())break;
//	if(IsCoefSame(*it_coef,coef_data) ) continue;
//	if((*inFace)==( *it_face->m_face)) {
//   		pitEntry->RemoveIncoming (inFace);
//    		pitEntry->RemoveCoef(*it_coef);
//		continue;
//		}
//      bool ok = it_face->m_face->Send (origPacket->Copy ());
//	
//      DidSendOutData (inFace, it_face->m_face, header, payload, origPacket, pitEntry);
//      NS_LOG_DEBUG ("Satisfy " << it_face->m_face);
//
//      if (!ok)
//        {
//          m_dropData (header, payload, it_face->m_face);
//          NS_LOG_DEBUG ("Cannot satisfy data to " << *it_face->m_face);
//        }
//     pitEntry->RemoveIncoming(it_face->m_face);
//     pitEntry->RemoveCoef(*it_coef);
//    }
//
//  // Remove all outgoing faces
//  pitEntry->ClearOutgoing ();
//
//  // Set pruning timout on PIT entry (instead of deleting the record)
//  m_pit->MarkErased (pitEntry);
}

void
ForwardingStrategy::DidReceiveSolicitedData (Ptr<Face> inFace,
                                             Ptr<const ContentObject> header,
                                             Ptr<const Packet> payload,
                                             Ptr<const Packet> origPacket,
                                             bool didCreateCacheEntry)
{
  // do nothing
}

void
ForwardingStrategy::DidReceiveUnsolicitedData (Ptr<Face> inFace,
                                               Ptr<const ContentObject> header,
                                               Ptr<const Packet> payload,
                                               Ptr<const Packet> origPacket,
                                               bool didCreateCacheEntry)
{
  // do nothing
}

void
ForwardingStrategy::WillSatisfyPendingInterest (Ptr<Face> inFace,
                                                Ptr<pit::Entry> pitEntry)
{
  pit::Entry::out_iterator out = pitEntry->GetOutgoing ().find (inFace);

  // If we have sent interest for this data via this face, then update stats.
  if (out != pitEntry->GetOutgoing ().end ())
    {
      pitEntry->GetFibEntry ()->UpdateFaceRtt (inFace, Simulator::Now () - out->m_sendTime);
    }

  m_satisfiedInterests (pitEntry);
}

bool
ForwardingStrategy::ShouldSuppressIncomingInterest (Ptr<Face> inFace,
                                                    Ptr<const Interest> header,
                                                    Ptr<const Packet> origPacket,
                                                    Ptr<pit::Entry> pitEntry)
{
  bool isNew = pitEntry->GetIncoming ().size () == 0 && pitEntry->GetOutgoing ().size () == 0;

  if (isNew) return false; // never suppress new interests

  bool isRetransmitted = m_detectRetransmissions && // a small guard
                         DetectRetransmittedInterest (inFace, header, origPacket, pitEntry);

  if (pitEntry->GetOutgoing ().find (inFace) != pitEntry->GetOutgoing ().end ())
    {
      NS_LOG_DEBUG ("Non duplicate interests from the face we have sent interest to. Don't suppress");
      // got a non-duplicate interest from the face we have sent interest to
      // Probably, there is no point in waiting data from that face... Not sure yet

      // If we're expecting data from the interface we got the interest from ("producer" asks us for "his own" data)
      // Mark interface YELLOW, but keep a small hope that data will come eventually.

      // ?? not sure if we need to do that ?? ...

      // pitEntry->GetFibEntry ()->UpdateStatus (inFace, fib::FaceMetric::NDN_FIB_YELLOW);
    }
  else
    if (!isNew && !isRetransmitted)
      {
        return true;
      }

  return false;
}

void
ForwardingStrategy::PropagateInterest (Ptr<Face> inFace,
                                       Ptr< Interest> header,
                                       Ptr<const Packet> origPacket,
                                       Ptr<pit::Entry> pitEntry)
{
  bool isRetransmitted = m_detectRetransmissions && // a small guard
                         DetectRetransmittedInterest (inFace, header, origPacket, pitEntry);

  pitEntry->AddIncoming (inFace, header->GetCoef());
  /// @todo Make lifetime per incoming interface
  pitEntry->UpdateLifetime (header->GetInterestLifetime ());

  bool propagated = DoPropagateInterest (inFace, header, origPacket, pitEntry);

  if (!propagated && isRetransmitted) //give another chance if retransmitted
    {
      // increase max number of allowed retransmissions
      pitEntry->IncreaseAllowedRetxCount ();

      // try again
      propagated = DoPropagateInterest (inFace, header, origPacket, pitEntry);
    }

  // if (!propagated)
  //   {
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //     NS_LOG_DEBUG ("+++ Not propagated ["<< header->GetName () <<"], but number of outgoing faces: " << pitEntry->GetOutgoing ().size ());
  //     NS_LOG_DEBUG ("++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  //   }

  // ForwardingStrategy will try its best to forward packet to at least one interface.
  // If no interests was propagated, then there is not other option for forwarding or
  // ForwardingStrategy failed to find it.
  if (!propagated && pitEntry->AreAllOutgoingInVain ())
    {
      DidExhaustForwardingOptions (inFace, header, origPacket, pitEntry);
    }

//std::cout<<"FW-interest:"<<header->GetName()<<"Success? "<<propagated<<std::endl;
}

bool
ForwardingStrategy::CanSendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const Interest> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
  if (outFace == inFace)
    {
      // NS_LOG_DEBUG ("Same as incoming");
      return false; // same face as incoming, don't forward
    }

  pit::Entry::out_iterator outgoing =
    pitEntry->GetOutgoing ().find (outFace);

  if (outgoing != pitEntry->GetOutgoing ().end ())
    {
      if (!m_detectRetransmissions)
        return false; // suppress
      else if (outgoing->m_retxCount >= pitEntry->GetMaxRetxCount ())
        {
          // NS_LOG_DEBUG ("Already forwarded before during this retransmission cycle (" <<outgoing->m_retxCount << " >= " << pitEntry->GetMaxRetxCount () << ")");
          return false; // already forwarded before during this retransmission cycle
        }
   }

  return true;
}


bool
ForwardingStrategy::TrySendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const Interest> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
  if (!CanSendOutInterest (inFace, outFace, header, origPacket, pitEntry))
    {
      return false;
    }

  pitEntry->AddOutgoing (outFace);

  //transmission
  Ptr<Packet> packetToSend = origPacket->Copy ();
  bool successSend = outFace->Send (packetToSend);
  if (!successSend)
    {
      m_dropInterests (header, outFace);
    }

  DidSendOutInterest (inFace, outFace, header, origPacket, pitEntry);

  return true;
}

void
ForwardingStrategy::DidSendOutInterest (Ptr<Face> inFace,
                                        Ptr<Face> outFace,
                                        Ptr<const Interest> header,
                                        Ptr<const Packet> origPacket,
                                        Ptr<pit::Entry> pitEntry)
{
  m_outInterests (header, outFace);
}

void
ForwardingStrategy::DidSendOutData (Ptr<Face> inFace,
                                    Ptr<Face> outFace,
                                    Ptr<const ContentObject> header,
                                    Ptr<const Packet> payload,
                                    Ptr<const Packet> origPacket,
                                    Ptr<pit::Entry> pitEntry)
{
  m_outData (header, payload, inFace == 0, outFace);
}

void
ForwardingStrategy::WillEraseTimedOutPendingInterest (Ptr<pit::Entry> pitEntry)
{
  m_timedOutInterests (pitEntry);
}

void
ForwardingStrategy::AddFace (Ptr<Face> face)
{
  // do nothing here
}

void
ForwardingStrategy::RemoveFace (Ptr<Face> face)
{
  // do nothing here
}

void
ForwardingStrategy::DidAddFibEntry (Ptr<fib::Entry> fibEntry)
{
  // do nothing here
}

void
ForwardingStrategy::WillRemoveFibEntry (Ptr<fib::Entry> fibEntry)
{
  // do nothing here
}

//added by zfx
uint64_t
ForwardingStrategy::GetCoef(Ptr<const ContentObject> header)
{
	list<string> prefix = header->GetName().GetComponents();
  list<string>::iterator  iter;
  int num = 0;
  string vod, nc, seg;
  for (iter = prefix.begin(); iter != prefix.end(); iter++, num++)
   {
    if( num == 3)
       return boost::lexical_cast<uint64_t>(*iter)%40+10;
  }
  return 0;
}
} // namespace ndn
} // namespace ns3
