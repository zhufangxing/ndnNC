/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of peking university, fangxing zhu
 *
 *
 * Author: fangxing zhu <zhufangxing@pku.edu.cn>
 */

#include "pinform.h"

#include "ns3/ndn-interest.h"
#include "ns3/ndn-pit.h"
#include "ns3/ndn-pit-entry.h"
#include "ns3/ndnSIM/utils/ndn-fw-hop-time-tag.h"
 #include "ns3/ndnSIM/utils/ndn-fw-hop-count-tag.h"
#include "ns3/random-variable.h"
#include "ns3/ndn-content-object.h"
#include "pinform-parameter.h"

#include "ns3/assert.h"
#include "ns3/log.h"

#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
namespace ll = boost::lambda;

//added by zfx , used in Pinform forwarding


namespace ns3 {
namespace ndn {
namespace fw {

NS_OBJECT_ENSURE_REGISTERED (Pinform);

LogComponent Pinform::g_log = LogComponent (Pinform::GetLogName ().c_str ());

std::string
Pinform::GetLogName ()
{
  return super::GetLogName ()+".Pinform";
}


const int LARGEINT = 0x7fffff;
const int m_maxInterest[2] = {0,5000};
// Note the parameters, such as m_QLearningPar/m_Sigema/m_baseProbability/m_factor, are defined in the file "pinform-parameter.h".
// const double m_QLearningPar = 0.5,m_Sigema = 0.15, m_baseProbability = 0.2;
// const float m_factor = 0.2;


struct FaceMetricByFace
{
  typedef fib::FaceMetricContainer::type::index<fib::i_face>::type
  type;
};

struct FaceMetricByQValue
{
  typedef fib::FaceMetricContainer::type::index<fib::i_qvalue>::type
  type;
};

struct FaceMetricByModifyP
{
  typedef fib::FaceMetricContainer::type::index<fib::i_probability>::type
  type;
};

TypeId
Pinform::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ndn::fw::Pinform")
    .SetGroupName ("Ndn")
    .SetParent <super> ()
    .AddConstructor <Pinform> ()
    ;
  return tid;
}

Pinform::Pinform ()
{
}

bool
Pinform::DoPropagateInterest (Ptr<Face> inFace,
                                Ptr< Interest> header,
                                Ptr<const Packet> origPacket,
                                Ptr<pit::Entry> pitEntry)
{

  static double Interestcount = 0.0;
  static double propaInterest = 0.0;//, propaIntInExplo = 0.0;
  Interestcount += 1;
  NS_LOG_FUNCTION (this << header->GetName ());

  int propagatedCount = 0;
  UniformVariable rand (0,std::numeric_limits<uint32_t>::max ());
  uint32_t randInt = rand.GetValue ();
  double randNum = (randInt%10000)/10000.0;
  double sum = 0.0;

  //test
  //selection probabilitiy assosicate with hops
  FwHopCountTag hopCountTag;
  origPacket->PeekPacketTag(hopCountTag);
  int hops = hopCountTag.Get();
  float factor = 1-hops*m_factor;
  if(factor<0) factor = 0;
  //cout<<"factor:"<<factor<<endl;
  randNum *= factor;
  //end test

  BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_probability> ())
    {
      if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) break;
      sum += metricFace.GetModifyProbability();
      //cout<<"DoPropagateInterest"<<sum<<" "<<randNum<<" "<<randInt<<endl;
      if (sum>= randNum) {
        if (!TrySendOutInterest (inFace, metricFace.GetFace (), header, origPacket, pitEntry))
        {
          propagatedCount+=BestRouteDoPropagateInterest(inFace, header, origPacket, pitEntry);
        }
        else
          propagatedCount+=1;
        break;
      }
    }

    if(propagatedCount == 0) ;//BestRouteDoPropagateInterest(inFace, header, origPacket, pitEntry);//cout<<"fail propagatedCount"<<propaInterest/Interestcount<<endl;
    else propaInterest+=1;
  // exploration phase, random choose a face to exploration
  if(pitEntry->GetFibEntry ()->m_phase == 0)
  {
    // propaIntInExplo += 1;
    //cout<<"exploration"<<propaIntInExplo/Interestcount<<endl;
    int count =0;
    BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ())
    {
      if(metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) break;
      if(!metricFace.GetFeasible()) count++;
    }
    int num = rand.GetValue();
    if(count == 0) return propagatedCount>0;
    num %=count;
    BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ())
    {
      if(metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) break;
      if(!metricFace.GetFeasible()) 
      {
        if(num != 0)
          num--;
        else  if (TrySendOutInterest (inFace, metricFace.GetFace (), header, origPacket, pitEntry))
        {
          propagatedCount++;
        }
      }

    }
  }
  NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
 return propagatedCount>0;

}

bool
Pinform::BestRouteDoPropagateInterest (Ptr<Face> inFace,
                                Ptr<const Interest> header,
                                Ptr<const Packet> origPacket,
                                Ptr<pit::Entry> pitEntry)
{
  NS_LOG_FUNCTION (this << header->GetName ());
  int propagatedCount = 0;
  BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_metric> ())
    {
      NS_LOG_DEBUG ("Trying " << boost::cref(metricFace));
      if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in front
        break;

      if (!TrySendOutInterest (inFace, metricFace.GetFace (), header, origPacket, pitEntry))
        {
          continue;
        }

      propagatedCount++;
      break; // do only once
    }

  //NS_LOG_INFO ("Propagated to " << propagatedCount << " faces");
  return propagatedCount > 0;
}

void Pinform::OnInterest (Ptr<Face> inFace,
                   Ptr<Interest> header,
                   Ptr<const Packet> origPacket)
{
  //get pit
  Ptr<pit::Entry> pitEntry = m_pit->Lookup(*header);
  if (pitEntry == 0)
    {
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

  //first start ? if yes, init Q value, P value
  FaceMetricByModifyP::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_probability> ().begin();
  if(metricFace == pitEntry->GetFibEntry ()->m_faces.get<fib::i_probability> ().end())
  {
    //do nothing
  }
  else if( metricFace->GetQVariable() == LARGEINT)
  {
    initValue(pitEntry->GetFibEntry());
  }
  else{
    //whether change pahses or not
    int phase = pitEntry->GetFibEntry ()->m_phase;
    if(phase ==1 or phase==0)
    {
      pitEntry->GetFibEntry ()->m_interestCountInThisPhase ++;
      if(pitEntry->GetFibEntry ()->m_interestCountInThisPhase > m_maxInterest[phase]) 
      {
        pitEntry->GetFibEntry ()->ChangePhase();
        pitEntry->GetFibEntry ()->m_interestCountInThisPhase = 0;
      }
    }
    else{
      NS_ASSERT_MSG(false, "wrong phase");
      //phase only could be 0 or 1, in wrong phase period, reset the phase
      pitEntry->GetFibEntry ()->m_phase = 0;
      pitEntry->GetFibEntry ()->m_interestCountInThisPhase = 0;
    }
  }

  //wait to add hopTimeTag, if the Interest match the cs

  FwHopTimeTag hopTimeTag;
  if(!origPacket->PeekPacketTag (hopTimeTag))
  {
    origPacket->AddPacketTag(hopTimeTag);
  }
  super::OnInterest(inFace, header, origPacket);
}



void  Pinform::OnData (Ptr<Face> face,
              Ptr<const ContentObject> header,
              Ptr<Packet> payload,
              Ptr<const Packet> origPacket)
{
  //get pit
  Ptr<pit::Entry> pitEntry = m_pit->Lookup(*header);  
  if (pitEntry == 0)
    {
      super::OnData(face, header, payload, origPacket);
      return;
    }


  //ns3::ndn::fib::FaceMetricContainer::type & m_faces = pitEntry->GetFibEntry ()->m_faces;
  FwHopTimeTag hopTimeTag;
  if(origPacket->PeekPacketTag (hopTimeTag))
  {
    uint64_t lastHopTime = hopTimeTag.GetHopTime();
    double QExpect = hopTimeTag.GetQExpect();
    double realQExpect = 0.0;
    // zfx WARN: may be not find the face, wait to deal
    FaceMetricByFace::type::iterator record = pitEntry->GetFibEntry ()->m_faces.get<fib::i_face> ().find (face);
    NS_ASSERT_MSG (record != pitEntry->GetFibEntry ()->m_faces.get<fib::i_face> ().end (),
                 "Update Qvariable can be performed only on existing faces of CcxnFibEntry");
    double newQV = record->GetQVariable()+ m_QLearningPar*(Simulator::Now ().GetMilliSeconds() - lastHopTime + QExpect);
    pitEntry->GetFibEntry ()->UpdateQVariable(face, newQV);

    // a exploration data ?
    if(record->GetFeasible()) // a data not in exploration phase
    // if(false)
    {
      double sumCountPerSec =0.0;
      BOOST_FOREACH (const fib::FaceMetric &metricFace, pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ())
      {
        if (metricFace.GetStatus () == fib::FaceMetric::NDN_FIB_RED || !metricFace.GetFeasible()) // all non-read faces are in front
          break;

        // init Q value
        if(metricFace.GetQVariable() == LARGEINT) {
          if(metricFace.GetRoutingCost() != 0)
            pitEntry->GetFibEntry ()->UpdateQVariable(metricFace.GetFace(), metricFace.GetRoutingCost());
          else pitEntry->GetFibEntry ()->UpdateQVariable(metricFace.GetFace(), LARGEINT);
        }

        sumCountPerSec += 1/metricFace.GetQVariable();
      }


      if(sumCountPerSec == 0)
      {
        // wait to deal with, maybe no face is feasible
        // Q value is set as a large num, avoiding sumCountPerSec be 0
        for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED || !metricFace->GetFeasible()) // all non-read faces are in front
            break;
          pitEntry->GetFibEntry()->UpdateModifyProbability(metricFace->GetFace(), 0.0);
          // m_faces.modify (metricFace,
          //             ll::bind (&fib::FaceMetric::SetProbability, ll::_1, 0.0));
        }

      }
      else for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
      {
        if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED || !metricFace->GetFeasible()) // all non-read faces are in front
          break;
        pitEntry->GetFibEntry ()->UpdateModifyProbability(metricFace->GetFace(), (1/metricFace->GetQVariable())/sumCountPerSec);
        realQExpect += metricFace->GetQVariable() * metricFace->GetModifyProbability();
        // cout<<"1GetModifyProbability"<<metricFace->GetQVariable()<<" "<<metricFace->GetProbability()<<" "<<metricFace->GetModifyProbability()<<endl;        
      }

      // whether to update QExpect value
      if(pitEntry->GetFibEntry ()->m_phase == 0 || pitEntry->GetFibEntry ()->m_QExpectInExplo == 0 || abs(pitEntry->GetFibEntry ()->m_QExpectInExplo - realQExpect)/pitEntry->GetFibEntry ()->m_QExpectInExplo > m_Sigema )
      {
        pitEntry->GetFibEntry ()->m_QExpectInExplo = realQExpect;
        if(pitEntry->GetFibEntry ()->m_phase == 1)
        {
          pitEntry->GetFibEntry ()->m_phase = 0;
          pitEntry->GetFibEntry ()->m_interestCountInThisPhase = 0;
        }
      }


    }// endIF --- exploitation


    else  //a data in exploration phase
    {
      double sumCountPerSec =0.0; // caculate all face
      double sumCountPerSec2 =0.0; //only caculate feasible face
      for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
      { 
        if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED ) // all non-read faces are in front
          break;

        // init Q value
        if(metricFace->GetQVariable() == 0 || metricFace->GetQVariable() == LARGEINT) {
          if(metricFace->GetRoutingCost() != 0)
            pitEntry->GetFibEntry ()->UpdateQVariable(metricFace->GetFace(), metricFace->GetRoutingCost());
          else pitEntry->GetFibEntry ()->UpdateQVariable(metricFace->GetFace(), LARGEINT);
        }

        sumCountPerSec += 1/metricFace->GetQVariable();
      }

      if(sumCountPerSec == 0)
      {
        // wait to deal with, maybe no face is feasible
        // Q value is set as a large num, avoiding sumCountPerSec be 0
        for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in front
            break;
          //metricFace->SetProbability(0.0);
          pitEntry->GetFibEntry()->UpdateProbability(metricFace->GetFace(), 0.0);
        }

      }
      else 
      { 
        for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED) // all non-read faces are in front
            break;
          pitEntry->GetFibEntry()->UpdateProbability(metricFace->GetFace(), (1/metricFace->GetQVariable())/sumCountPerSec);
          if(metricFace->GetProbability() > m_baseProbability)
          {
            pitEntry->GetFibEntry()->UpdateFeasible(metricFace->GetFace(), true);
            sumCountPerSec2 += 1/metricFace->GetQVariable();
          }
          else
          {
            pitEntry->GetFibEntry()->UpdateFeasible(metricFace->GetFace(), false);
            pitEntry->GetFibEntry ()->UpdateModifyProbability(metricFace->GetFace(),0.0);
          }
        } 

        // wait to deal with, if sumCountPerSce2 == 0


        for (FaceMetricByQValue::type::iterator metricFace = pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != pitEntry->GetFibEntry ()->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if (metricFace->GetStatus () == fib::FaceMetric::NDN_FIB_RED || !metricFace->GetFeasible()) // all non-read faces are in front
            break;
          pitEntry->GetFibEntry ()->UpdateModifyProbability(metricFace->GetFace(), (1/metricFace->GetQVariable())/sumCountPerSec2);
          realQExpect += metricFace->GetQVariable() * metricFace->GetModifyProbability();
          //cout<<"3GetModifyProbability"<<metricFace->GetQVariable()<<" "<<metricFace->GetProbability()<<" "<<metricFace->GetModifyProbability()<<endl;        
        } 

        // update Q expect value
        pitEntry->GetFibEntry ()->m_QExpectInExplo = realQExpect;
      }

    }//else -- exploration

    Ptr<Packet> newPacket = origPacket->Copy();
    //newPacket->RemovePacketTag (hopTimeTag);
    NS_ASSERT_MSG (newPacket->RemovePacketTag (hopTimeTag), "fail remove hopTimeTag");

    //hopTimeTag.Update(realQExpect);
    FwHopTimeTag hopTimeTag2(realQExpect);
    newPacket->AddPacketTag(hopTimeTag2);
    super::OnData(face, header, payload, newPacket);
  }// endif --- get a hopTimeTag

  else{
    Ptr<Packet> newPacket = origPacket->Copy();
    newPacket->RemovePacketTag (hopTimeTag);
    FwHopTimeTag hopTimeTag2(pitEntry->GetFibEntry()->m_QExpectInExplo);
    newPacket->AddPacketTag(hopTimeTag2);
    super::OnData(face, header, payload, newPacket);
  }

}

void
Pinform::initValue(Ptr<fib::Entry> fibEntry)
{
  //out<<"init"<<endl;
  double sumCountPerSec = 0.0;
  double sumCountPerSec2 = 0.0;
  double realQExpect = 0.0;
  for (FaceMetricByQValue::type::iterator metricFace = fibEntry->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != fibEntry->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if(metricFace->GetRoutingCost() == 0)
            fibEntry->UpdateQVariable(metricFace->GetFace(), LARGEINT);
          else
            fibEntry->UpdateQVariable(metricFace->GetFace(), metricFace->GetRoutingCost());
          sumCountPerSec += 1/metricFace->GetQVariable();
          //cout<<"GetQVariable"<<metricFace->GetQVariable()<<" "<<metricFace->GetRoutingCost()<<endl;
        }
  for (FaceMetricByQValue::type::iterator metricFace = fibEntry->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != fibEntry->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          fibEntry->UpdateProbability(metricFace->GetFace(), (1/metricFace->GetQVariable())/sumCountPerSec);
          if(metricFace->GetProbability()>m_baseProbability)
            sumCountPerSec2 += 1/metricFace->GetQVariable();
        }
  for (FaceMetricByQValue::type::iterator metricFace = fibEntry ->m_faces.get<fib::i_qvalue> ().begin();
              metricFace != fibEntry ->m_faces.get<fib::i_qvalue> ().end();
              metricFace ++)
        {
          if(metricFace->GetProbability()>m_baseProbability)
          {
            fibEntry->UpdateModifyProbability(metricFace->GetFace(), (1/metricFace->GetQVariable())/sumCountPerSec2);
            fibEntry->UpdateFeasible(metricFace->GetFace(), true);            
            realQExpect += metricFace->GetModifyProbability()*metricFace->GetQVariable();
          }
          else
          {
            fibEntry->UpdateFeasible(metricFace->GetFace(), false);
            fibEntry->UpdateModifyProbability(metricFace->GetFace(), 0.0);
          }
          // cout<<"2GetModifyProbability"<<metricFace->GetQVariable()<<" "<<metricFace->GetProbability()<<" "<<metricFace->GetModifyProbability()<<endl;
        }
  fibEntry->m_phase = 1;
  fibEntry->m_interestCountInThisPhase = 0;
  fibEntry->m_QExpectInExplo = realQExpect;

}

} // namespace fw
} // namespace ndn
} // namespace ns3