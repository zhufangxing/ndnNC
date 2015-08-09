/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_CONTENT_STORE_WITH_FRESHNESS_H_
#define NDN_CONTENT_STORE_WITH_FRESHNESS_H_

#include "content-store-impl.h"

#include "../../utils/trie/multi-policy.h"

#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list.hpp>

#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/traced-callback.h>

namespace ns3 {
namespace ndn {
namespace ndnSIM{


// add by zfx
// This file is modified by following two files: custom_policies/freshness_police.h  and content-store-with-freshness.h
// LifetimeBaseGreedy_policy_traits : modified from custom_policies/freshness_police.h 
// modify lookup and update functions.  

/**
 * @brief Traits for freshness policy
 */
struct mLifetimeBaseGreedy_policy_traits
{
  /// @brief Name that can be used to identify the policy (for NS-3 object model and logging)
  static std::string GetName () { return "Freshness"; }

  struct policy_hook_type : public boost::intrusive::set_member_hook<> { Time timeWhenShouldExpire; };

  template<class Container>
  struct container_hook
  {
    typedef boost::intrusive::member_hook< Container,
                                           policy_hook_type,
                                           &Container::policy_hook_ > type;
  };

  template<class Base,
           class Container,
           class Hook>
  struct policy
  {
    static Time& get_freshness (typename Container::iterator item)
    {
      return static_cast<typename policy_container::value_traits::hook_type*>
        (policy_container::value_traits::to_node_ptr(*item))->timeWhenShouldExpire;
    }

    static const Time& get_freshness (typename Container::const_iterator item)
    {
      return static_cast<const typename policy_container::value_traits::hook_type*>
        (policy_container::value_traits::to_node_ptr(*item))->timeWhenShouldExpire;
    }

    template<class Key>
    struct MemberHookLess
    {
      bool operator () (const Key &a, const Key &b) const
      {
        return get_freshness (&a) < get_freshness (&b);
      }
    };

    typedef boost::intrusive::multiset< Container,
                                   boost::intrusive::compare< MemberHookLess< Container > >,
                                   Hook > policy_container;


    class type : public policy_container
    {
    public:
      typedef policy policy_base; // to get access to get_freshness methods from outside
      typedef Container parent_trie;

      type (Base &base)
        : base_ (base)
        , max_size_ (100)
      {
      }

      inline void
      update (typename parent_trie::iterator item)
      {
        // do nothing
        Time freshness = item->payload ()->GetHeader ()->GetFreshness ();
        if (!freshness.IsZero ())
          {
            get_freshness (item) = Simulator::Now () + freshness;

            // push item only if freshness is non zero. otherwise, this payload is not controlled by the policy
            // note that .size() on this policy would return only number of items with non-infinite freshness policy
            policy_container::push_back (*item);
          }
      }

      inline bool
      insert (typename parent_trie::iterator item)
      {
        // get_time (item) = Simulator::Now ();
        Time freshness = item->payload ()->GetHeader ()->GetFreshness ();
        if (!freshness.IsZero ())
          {
            get_freshness (item) = Simulator::Now () + freshness;

            // push item only if freshness is non zero. otherwise, this payload is not controlled by the policy
            // note that .size() on this policy would return only number of items with non-infinite freshness policy
            policy_container::push_back (*item);
          }

        return true;
      }

      inline void
      lookup (typename parent_trie::iterator item)
      {
        Time freshness = item->payload ()->GetHeader ()->GetFreshness ();
        if (!freshness.IsZero ())
          {
            get_freshness (item) = Simulator::Now () + freshness;

            // push item only if freshness is non zero. otherwise, this payload is not controlled by the policy
            // note that .size() on this policy would return only number of items with non-infinite freshness policy
            policy_container::push_back (*item);
          }
      }

      inline void
      erase (typename parent_trie::iterator item)
      {
        if (!item->payload ()->GetHeader ()->GetFreshness ().IsZero ())
          {
            // erase only if freshness is non zero (otherwise an item is not in the policy
            policy_container::erase (policy_container::s_iterator_to (*item));
          }
      }

      inline void
      clear ()
      {
        policy_container::clear ();
      }

      inline void
      set_max_size (size_t max_size)
      {
        max_size_ = max_size;
      }

      inline size_t
      get_max_size () const
      {
        return max_size_;
      }

    private:
      type () : base_(*((Base*)0)) { };

    private:
      Base &base_;
      size_t max_size_;
    };
  };
};
} // namespace ndnSIM
} // namespace ndn
} // namespace ns3




// zfx
// modified from content-store-with-freshness.h
namespace ns3 {
namespace ndn {
namespace cs {


template<class Policy>
class ContentStoreWithLifetimeBasedGreedy :
    public ContentStoreImpl< ndnSIM::multi_policy_traits< boost::mpl::vector2< Policy, ndnSIM::mLifetimeBaseGreedy_policy_traits > > >
{
public:
  typedef ContentStoreImpl< ndnSIM::multi_policy_traits< boost::mpl::vector2< Policy, ndnSIM::mLifetimeBaseGreedy_policy_traits > > > super;

  typedef typename super::policy_container::template index<1>::type freshness_policy_container;

  static TypeId
  GetTypeId ();

  virtual inline void
  Print (std::ostream &os) const;

  virtual inline bool
  Add (Ptr<const ContentObject> header, Ptr<const Packet> packet);

private:
  inline void
  CleanExpired ();

  inline void
  RescheduleCleaning ();

private:
  static LogComponent g_log; ///< @brief Logging variable

  EventId m_cleanEvent;
  Time m_scheduledCleaningTime;
};

//////////////////////////////////////////
////////// Implementation ////////////////
//////////////////////////////////////////


template<class Policy>
LogComponent
ContentStoreWithLifetimeBasedGreedy< Policy >::g_log = LogComponent (("ndn.cs.LifetimeBasedGreedy." + Policy::GetName ()).c_str ());


template<class Policy>
TypeId
ContentStoreWithLifetimeBasedGreedy< Policy >::GetTypeId ()
{
  static TypeId tid = TypeId (("ns3::ndn::cs::LifetimeBasedGreedy::"+Policy::GetName ()).c_str ())
    .SetGroupName ("Ndn")
    .SetParent<super> ()
    .template AddConstructor< ContentStoreWithLifetimeBasedGreedy< Policy > > ()

    // trace stuff here
    ;

  return tid;
}

//modified by zfx
template<class Policy>
inline bool
ContentStoreWithLifetimeBasedGreedy< Policy >::Add (Ptr<const ContentObject> header, Ptr<const Packet> packet)
{
  if (!m_cleanEvent.IsRunning ())
    {
      CleanExpired ();
    }
  if (!super::isFull())
  {
    bool ok = super::Add (header, packet);
    if (!ok) return false;

    NS_LOG_DEBUG (header->GetName () << " added to cache");  
    return true;    
  }
  
  return false;  
}

template<class Policy>
inline void
ContentStoreWithLifetimeBasedGreedy< Policy >::RescheduleCleaning ()
{
  const freshness_policy_container &freshness = this->getPolicy ().template get<freshness_policy_container> ();

  if (freshness.size () > 0)
    {
      Time nextStateTime = freshness_policy_container::policy_base::get_freshness (&(*freshness.begin ()));

      if (m_scheduledCleaningTime.IsZero () || // if not yet scheduled
          m_scheduledCleaningTime > nextStateTime) // if new item expire sooner than already scheduled
        {
          if (m_cleanEvent.IsRunning ())
            {
              Simulator::Remove (m_cleanEvent); // just canceling would not clean up list of events
            }

          // NS_LOG_DEBUG ("Next event in: " << (nextStateTime - Now ()).ToDouble (Time::S) << "s");
          m_cleanEvent = Simulator::Schedule (nextStateTime - Now (), &ContentStoreWithLifetimeBasedGreedy< Policy >::CleanExpired, this);
          m_scheduledCleaningTime = nextStateTime;
        }
    }
  else
    {
      if (m_cleanEvent.IsRunning ())
        {
          Simulator::Remove (m_cleanEvent); // just canceling would not clean up list of events
        }
    }
}


template<class Policy>
inline void
ContentStoreWithLifetimeBasedGreedy< Policy >::CleanExpired ()
{
  freshness_policy_container &freshness = this->getPolicy ().template get<freshness_policy_container> ();

  // NS_LOG_LOGIC (">> Cleaning: Total number of items:" << this->getPolicy ().size () << ", items with freshness: " << freshness.size ());
  Time now = Simulator::Now ();

  while (!freshness.empty ())
    {
      typename freshness_policy_container::iterator entry = freshness.begin ();

      if (freshness_policy_container::policy_base::get_freshness (&(*entry)) <= now) // is the record stale?
        {
          super::erase (&(*entry));
        }
      else
        break; // nothing else to do. All later records will not be stale
    }
  // NS_LOG_LOGIC ("<< Cleaning: Total number of items:" << this->getPolicy ().size () << ", items with freshness: " << freshness.size ());

  m_scheduledCleaningTime = Time ();
  RescheduleCleaning ();
}

template<class Policy>
void
ContentStoreWithLifetimeBasedGreedy< Policy >::Print (std::ostream &os) const
{
  // const freshness_policy_container &freshness = this->getPolicy ().template get<freshness_policy_container> ();

  for (typename super::policy_container::const_iterator item = this->getPolicy ().begin ();
       item != this->getPolicy ().end ();
       item++)
    {
      Time ttl = freshness_policy_container::policy_base::get_freshness (&(*item)) - Simulator::Now ();
      os << item->payload ()->GetName () << "(left: " << ttl.ToDouble (Time::S) << "s)" << std::endl;
    }
}



} // namespace cs
} // namespace ndn
} // namespace ns3

#endif // NDN_CONTENT_STORE_WITH_FRESHNESS_H_
