#ifndef NetworkCodingApp_H_
#define NetworkCodingApp_H_

#include "ns3/ndn-app.h"
#include "ns3/ndn-name.h"

namespace ns3 {
namespace ndn {
class NetworkCodingApp : public ndn::App
{
public:
  // register NS-3 type "CustomApp"
  static TypeId GetTypeId ();
  
  // (overridden from ndn::App) Processing upon start of the application
  virtual void StartApplication ();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void StopApplication ();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  //virtual void
  //OnInterest (Ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  //virtual void
  //OnData (Ptr<const ndn::Data> contentObject);
  
  virtual void OnInterest (const Ptr<const ndn::InterestHeader> &interest, Ptr<Packet> origPacket);

	// (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void	OnContentObject (const Ptr<const ndn::ContentObjectHeader> &contentObject,
				Ptr<Packet> payload);


//added by zfx
  bool
  IsCoefSame(uint64_t coef_pit, uint64_t coef_data);
private:
  void  SendInterest ();
  Name m_prefix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;
};

} // namespace ndn
} // namespace ns3

#endif // NETWROKCODING_APP_H_
