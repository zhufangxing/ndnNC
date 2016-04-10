/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Ilya Moiseenko <iliamo@cs.ucla.edu>
 */

#include "ndn-consumer-nc.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

#include "ns3/ndn-app-face.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/ndnSIM/utils/ndn-rtt-mean-deviation.h"

#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include "ns3/names.h"

namespace ll = boost::lambda;

NS_LOG_COMPONENT_DEFINE ("ndn.ConsumerNC");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED (ConsumerNC);

TypeId
ConsumerNC::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::ConsumerNC")
    .SetGroupName ("Ndn")
    .SetParent<App> ()
    .AddAttribute ("StartSeq", "Initial sequence number",
                   IntegerValue (0),
                   MakeIntegerAccessor(&ConsumerNC::m_seq),
                   MakeIntegerChecker<int32_t>())

    .AddAttribute ("Prefix","Name of the Interest",
                   StringValue ("/"),
                   MakeNameAccessor (&ConsumerNC::m_interestName),
                   MakeNameChecker ())
    .AddAttribute ("LifeTime", "LifeTime for interest packet",
                   StringValue ("2s"),
                   MakeTimeAccessor (&ConsumerNC::m_interestLifeTime),
                   MakeTimeChecker ())

    .AddAttribute ("RetxTimer",
                   "Timeout defining how frequent retransmission timeouts should be checked",
                   StringValue ("50ms"),
                   MakeTimeAccessor (&ConsumerNC::GetRetxTimer, &ConsumerNC::SetRetxTimer),
                   MakeTimeChecker ())

    .AddTraceSource ("LastRetransmittedInterestDataDelay", "Delay between last retransmitted Interest and received Data",
                     MakeTraceSourceAccessor (&ConsumerNC::m_lastRetransmittedInterestDataDelay))

    .AddTraceSource ("FirstInterestDataDelay", "Delay between first transmitted Interest and received Data",
                     MakeTraceSourceAccessor (&ConsumerNC::m_firstInterestDataDelay))
    ;

  return tid;
}

ConsumerNC::ConsumerNC ()
  : m_rand (0, std::numeric_limits<uint32_t>::max ())
  , m_seq (0)
  , m_seqMax (0) // don't request anything
{
  NS_LOG_FUNCTION_NOARGS ();

  m_rtt = CreateObject<RttMeanDeviation> ();

}

void
ConsumerNC::SetRetxTimer (Time retxTimer)
{
  m_retxTimer = retxTimer;
  if (m_retxEvent.IsRunning ())
    {
      // m_retxEvent.Cancel (); // cancel any scheduled cleanup events
      Simulator::Remove (m_retxEvent); // slower, but better for memory
    }

  // schedule even with new timeout
  m_retxEvent = Simulator::Schedule (m_retxTimer,
                                     &ConsumerNC::CheckRetxTimeout, this);
}

Time
ConsumerNC::GetRetxTimer () const
{
  return m_retxTimer;
}

void
ConsumerNC::CheckRetxTimeout ()
{
  Time now = Simulator::Now ();

  Time rto = m_rtt->RetransmitTimeout ();
 //  NS_LOG_DEBUG ("Current RTO: " << rto.ToDouble (Time::S) << "s");
  //std::cout<< "Current RTO: " << rto.ToDouble (Time::S) << "s"<< std::endl;
  //rto = 2s;
  while (!m_seqTimeouts.empty ())
    {
      SeqTimeoutsContainer::index<i_timestamp>::type::iterator entry =
        m_seqTimeouts.get<i_timestamp> ().begin ();
      if (entry->time + rto <= now ) // timeout expired?
      //if (entry->time + rto <= now || (now.ToDouble(Time::S)-entry->time.ToDouble(Time::S))>2) // modefied by zfx
        {
          uint32_t seqNo = entry->seq;
          m_seqTimeouts.get<i_timestamp> ().erase (entry);
          OnTimeout (seqNo);
        }
      else
        break; // nothing else to do. All later packets need not be retransmitted
    }

  m_retxEvent = Simulator::Schedule (m_retxTimer,
                                     &ConsumerNC::CheckRetxTimeout, this);
}

// Application Methods
void
ConsumerNC::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION_NOARGS ();

  // do base stuff
  App::StartApplication ();

  ScheduleNextPacket ();
}

void
ConsumerNC::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION_NOARGS ();

  // cancel periodic packet generation
  Simulator::Cancel (m_sendEvent);

  // cleanup base stuff
  App::StopApplication ();
}

void
ConsumerNC::SendPacket ()
{
  uint32_t blocksNum=4;
  uint32_t heap=500;	//eg, 1, 1+heap, 1+2*heap, 1+3*heap  are coeded in a segment
  uint32_t seqScop=blocksNum*heap;
  if (!m_active) return;

  NS_LOG_FUNCTION_NOARGS ();

  uint32_t seq=std::numeric_limits<uint32_t>::max (); //invalid
 
/* trassmission without same sgment blocks 
   std::set<uint32_t>::iterator it = m_retxSeqs.begin ();
  while (it != m_retxSeqs.end ())
    {
      seq = *it;
1//added by zfx
	bool flag_tx = true;	// whether or not  trassmission the seq
  	uint32_t seq_base=(seq/40)*40;
  	uint32_t seq_unit=seq%10;
	for(uint32_t i; i< blocksNum; i++)
	{
		SeqTimeoutsContainer::iterator entry = m_seqTimeouts.find (seq_base+i*10+seq_unit);
   		if (entry == m_seqTimeouts.end ()) continue;
		flag_tx = false;
		it++;
	 	break;
	}
	if(flag_tx)
		{
      		m_retxSeqs.erase (it);
		break;
		}
     // RetxSeqsContainer::iterator entry = m_arrivedData.find (seq);
     // if (entry == m_arrivedData.end ()) continue;
    }

  if (seq == std::numeric_limits<uint32_t>::max ())
    {
      if (m_seqMax != std::numeric_limits<uint32_t>::max ())
        {
          if (m_seq >= m_seqMax)
            {
		if( m_retxSeqs.size()) ScheduleNextPacket ();
		//if( m_retxSeqs.size()) SendPacket();
              return; // we are totally done
            }
        }

      seq = m_seq++;
	//a new seq, if it is has transimitted block in the same segment, it will add to the retxSeq.
	bool flag_tx = true;	// whether or not  trassmission the seq
  	uint32_t seq_base=(seq/40)*40;
  	uint32_t seq_unit=seq%10;
	for(uint32_t i; i< blocksNum; i++)
	{
		SeqTimeoutsContainer::iterator entry = m_seqTimeouts.find (seq_base+i*10+seq_unit);
   		if (entry == m_seqTimeouts.end ()) continue;
		flag_tx = false;
	 	break;
	}
	if(!flag_tx)
		{
		m_retxSeqs.insert(m_retxSeqs.end(),seq);
		return ScheduleNextPacket ();
		//return SendPacket();
		}
    }
*/  //transmission without same segment blocks ---zfx---failed----2014-12-7-----------


while (m_retxSeqs.size ())
    {
      seq = *m_retxSeqs.begin ();
      m_retxSeqs.erase (m_retxSeqs.begin ());
      break;
    }

  if (seq == std::numeric_limits<uint32_t>::max ())
    {
      if (m_seqMax != std::numeric_limits<uint32_t>::max ())
        {
          if (m_seq >= m_seqMax)
            {
              return; // we are totally done
            }
        }

      seq = m_seq++;
    }
//check arrived data to get coef to attached at Interest
//added by zfx 2014.11.30
 // std::string coef;
  uint64_t coef_t=0;
  uint32_t seq_base=(seq/seqScop)*seqScop;
  uint32_t seq_unit=seq%heap;
  for(uint32_t i=0; i<blocksNum ; i++)
  {
	uint32_t seq_t = seq_base+i*heap+seq_unit;
  	SeqArrivedContainer::iterator entry = m_arrivedData.get<i_seq>().find (seq_t);
 	if(entry!= m_arrivedData.end()){
//std::cout<<"coef_t:"<<coef_t<<" seq_t:"<<seq_t;
	coef_t=coef_t*100+entry->coef;
//std::cout<<" coef_t:"<<coef_t<<std::endl;
	}
  }
uint64_t coef=coef_t;
  Ptr<Name> nameWithSequence = Create<Name> (m_interestName);

//seq=11;//zfx test
  (*nameWithSequence) (seq);
 
  Interest interestHeader;
  interestHeader.SetNonce               (m_rand.GetValue ());
  interestHeader.SetName                (nameWithSequence);
  interestHeader.SetInterestLifetime    (m_interestLifeTime);
  interestHeader.SetCoef                (coef); 
  interestHeader.SetPathNum             (2); 
  //interestHeader.SetCoef                ("[0,1,3,4,5,6,7,8,2,19]"); 
  // NS_LOG_INFO ("Requesting Interest: \n" << interestHeader);
  NS_LOG_INFO ("> Interest for " << seq);
  //std::cout << "Interest for " << interestHeader.GetName()<<std::endl;
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (interestHeader);
  NS_LOG_DEBUG ("Interest packet size: " << packet->GetSize ());

  WillSendOutInterest (seq);  

  FwHopCountTag hopCountTag;
  packet->AddPacketTag (hopCountTag);
 // std::cout <<GetNode()->GetId()<< "Send Interest Coef: "<< interestHeader.GetCoef()<<"Name:"<<interestHeader.GetName()<<std::endl;
  m_transmittedInterests (&interestHeader, this, m_face);
  m_protocolHandler (packet);

  ScheduleNextPacket ();
}

///////////////////////////////////////////////////
//          Process incoming packets             //
///////////////////////////////////////////////////


void
ConsumerNC::OnContentObject (const Ptr<const ContentObject> &contentObject,
                               Ptr<Packet> payload)
{
  uint32_t blocksNum=4;
  uint32_t heap=500;	//eg, 1, 1+heap, 1+2*heap, 1+3*heap  are coeded in a segment
  uint32_t seqScop=blocksNum*heap;
  if (!m_active) return;

  App::OnContentObject (contentObject, payload); // tracing inside

  NS_LOG_FUNCTION (this << contentObject << payload);

  // NS_LOG_INFO ("Received content object: " << boost::cref(*contentObject));

//  uint32_t seq = boost::lexical_cast<uint32_t> (contentObject->GetName ().GetComponents ().back ());
  uint32_t seq=contentObject->GetSeq();
  NS_LOG_INFO ("< DATA for " << seq);
  SeqArrived arrivedSeq=SeqArrived(seq, contentObject->GetCoef());
  m_arrivedData.insert (arrivedSeq); //added by zfx
  //std::cout << "arrivedData: "<<seq;
  
  //std::cout <<"Node:"<<GetNode()->GetId()<<"  ConsumerNC:Data for content object:"<<*contentObject<<" payload:"<< *payload <<" ndn-ConsumerNC.cc line-256"<<std::endl;
  //added by zfx
    uint32_t seq_base=(seq/seqScop)*seqScop;
    uint32_t seq_unit=seq%heap;
    for(uint32_t i=0;i<blocksNum; i++){
    uint32_t seq_t =seq_base+i*heap+seq_unit;
    SeqTimeoutsContainer::iterator entry = m_seqTimeouts.get<i_seq>().find (seq_t);
    if (entry == m_seqTimeouts.end ()) continue;
	seq=seq_t;
	 break;
    }
  
  int hopCount = -1;
  FwHopCountTag hopCountTag;
  if (payload->RemovePacketTag (hopCountTag))
    {
      hopCount = hopCountTag.Get ();
    }

  SeqTimeoutsContainer::iterator entry = m_seqLastDelay.get<i_seq>().find (seq);
  if (entry != m_seqLastDelay.end ())
    {
      m_lastRetransmittedInterestDataDelay (this, seq, Simulator::Now () - entry->time, hopCount);
    }

  entry = m_seqFullDelay.get<i_seq>().find (seq);
  if (entry != m_seqFullDelay.end ())
    {
      m_firstInterestDataDelay (this, seq, Simulator::Now () - entry->time, m_seqRetxCounts[seq], hopCount);
    }

  m_seqRetxCounts.erase (seq);
  m_seqFullDelay.erase (seq);
  m_seqLastDelay.erase (seq);

  m_seqTimeouts.erase (seq);
  m_retxSeqs.erase (seq);
  //m_arrivedData.insert (seq); //added by zfx
  m_rtt->AckSeq (SequenceNumber32 (seq));
  //std::cout <<"Node:"<<GetNode()->GetId()<<"  ConsumerNC:Data for "<< contentObject->GetName()<<" Size:"<<payload->GetSize() << " ndn-ConsumerNC.cc line-285"<<std::endl;
  //const unsigned char *buff = new unsigned char[1024];
  //buff = payload->PeekData(); 
  //std::cout << " arrivedData: "<<seq<<std::endl;
}

void
ConsumerNC::OnNack (const Ptr<const Interest> &interest, Ptr<Packet> origPacket)
{
  if (!m_active) return;

  App::OnNack (interest, origPacket); // tracing inside

  // NS_LOG_DEBUG ("Nack type: " << interest->GetNack ());

  // NS_LOG_FUNCTION (interest->GetName ());

  // NS_LOG_INFO ("Received NACK: " << boost::cref(*interest));
  uint32_t seq = boost::lexical_cast<uint32_t> (interest->GetName ().GetComponents ().back ());
  NS_LOG_INFO ("< NACK for " << seq);
   //std::cout << Simulator::Now ().ToDouble (Time::S) << "s -> " << "NACK for " << seq << "\n";

  // put in the queue of interests to be retransmitted
  // NS_LOG_INFO ("Before: " << m_retxSeqs.size ());
  m_retxSeqs.insert (seq);
  // NS_LOG_INFO ("After: " << m_retxSeqs.size ());
  m_seqTimeouts.erase (seq);

  m_rtt->IncreaseMultiplier ();             // Double the next RTO ??
  ScheduleNextPacket ();
}

void
ConsumerNC::OnTimeout (uint32_t sequenceNumber)
{
  NS_LOG_FUNCTION (sequenceNumber);
  // std::cout << Simulator::Now () << ", TO: " << sequenceNumber << ", current RTO: " << m_rtt->RetransmitTimeout ().ToDouble (Time::S) << "s\n";

  m_rtt->IncreaseMultiplier ();             // Double the next RTO
  m_rtt->SentSeq (SequenceNumber32 (sequenceNumber), 1); // make sure to disable RTT calculation for this sample
  m_retxSeqs.insert (sequenceNumber);
  ScheduleNextPacket ();
}

void
ConsumerNC::WillSendOutInterest (uint32_t sequenceNumber)
{
  NS_LOG_DEBUG ("Trying to add " << sequenceNumber << " with " << Simulator::Now () << ". already " << m_seqTimeouts.size () << " items");

  m_seqTimeouts.insert (SeqTimeout (sequenceNumber, Simulator::Now ()));
  m_seqFullDelay.insert (SeqTimeout (sequenceNumber, Simulator::Now ()));

  m_seqLastDelay.erase (sequenceNumber);
  m_seqLastDelay.insert (SeqTimeout (sequenceNumber, Simulator::Now ()));

  m_seqRetxCounts[sequenceNumber] ++;

  m_rtt->SentSeq (SequenceNumber32 (sequenceNumber), 1);
}


} // namespace ndn
} // namespace ns3
