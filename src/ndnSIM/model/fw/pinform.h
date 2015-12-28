/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 University of peking university, fangxing zhu
 *
 *
 * Author: fangxing zhu <zhufangxing@pku.edu.cn>
 */


#ifndef NDNSIM_Pinform_H
#define NDNSIM_Pinform_H

#include "green-yellow-red.h"
#include "ns3/log.h"

namespace ns3 {
namespace ndn {
namespace fw {

/**
 * \ingroup ndn
 * \brief Best route strategy
 */
class Pinform :
    public GreenYellowRed
{
private:
  typedef GreenYellowRed super;

public:
  static TypeId
  GetTypeId ();

  /**
   * @brief Helper function to retrieve logging name for the forwarding strategy
   */
  static std::string
  GetLogName ();
  
  /**
   * @brief Default constructor
   */
  Pinform ();
        
  // from super
  virtual bool
  DoPropagateInterest (Ptr<Face> incomingFace,
                       Ptr<Interest> header,
                       Ptr<const Packet> origPacket,
                       Ptr<pit::Entry> pitEntry);
  bool
  BestRouteDoPropagateInterest (Ptr<Face> inFace,
                                Ptr<const Interest> header,
                                Ptr<const Packet> origPacket,
                                Ptr<pit::Entry> pitEntry);
  void OnInterest (Ptr<Face> inFace,
                   Ptr<Interest> header,
                   Ptr<const Packet> origPacket);
  void  OnData (Ptr<Face> face,
              Ptr<const ContentObject> header,
              Ptr<Packet> payload,
              Ptr<const Packet> origPacket);
  void initValue(Ptr<fib::Entry> fibEntry);

protected:
  static LogComponent g_log;
};

} // namespace fw
} // namespace ndn
} // namespace ns3

#endif // NDNSIM_BEST_ROUTE_H
