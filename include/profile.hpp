#include <eosio/eosio.hpp>

using namespace std;
using namespace eosio;


CONTRACT profile : public contract {
  public:
    using contract::contract;
  
  struct badge_count {
    name badge;
    uint16_t count;
  };

  /*
  * Create a new badge.
  *
  * This action creates a new badge.
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param badge is name of badge.
  * @param parentbadge is vector of parents of this badge. when this badge is allocated to an account, parent badges are  allocated automatically.
  * @param ipfsimage is stringified JSON with ipfs urls for images of diff dimension.
  * @param details is placeholder for any additional detail
  * @return no return value.
  */
  ACTION createsimple (name org, name badge, vector<name> parentbadge, string ipfsimage, string details);


  /*
  * Create a new gotcha badge. A gotcha badge can be claimed once every fixed duration (cycle_length).
  * Total balance for a claimer can not exceed max_cap
  *
  * This action creates a new gotcha badge.
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param badge is name of badge.
  * @param starttime is first time when this badge is available for claim
  * @param cycle_length 
  * @param max_cap 
  * @param ipfsimage is stringified JSON with ipfs urls for images of diff dimension.
  * @param details is placeholder for any additional detail
  * @return no return value.
  */
  ACTION creategotcha (name org, name badge, time_point starttime, uint64_t cycle_length, uint8_t max_cap, string ipfsimage, string details);

  /*
  * Claim gotcha badges for current cycle.
  *
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param account is claimer.
  * @return no return value.
  */

  ACTION claimgotcha (name org, name account);


  /*
  * Give gotcha badges to a member.
  *
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param from .
  * @param to .
  * @param amount .
  * @param memo .
  * @return no return value.
  */
  ACTION givegotcha (name org, name from, name to, uint8_t amount, string memo );

  /*
  * Give simple badge to a member.
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param to .
  * @param badge .
  * @param memo .
  * @return no return value.
  */
  ACTION givesimple (name org, name to, name badge, string memo );


  /*
  * Rollup combination of existing badges to receive a new badge.
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param account .
  * @param badge is the new badge recieved
  * @param existing_badges is condition represented by existing badge x count vector
  * @return no return value.
  */
  ACTION rollup (name org, name account, name badge, vector<badge_count> existing_badges);

  private:

    // scoped by org
    TABLE simplebadge {
      name badge;
      vector<name> parentbadge;
      string ipfsimage;
      string details;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("simplebadge"), simplebadge> simplebadge_table;

    // scoped by org
    TABLE gotchabadge {
      name badge;
      time_point starttime;
      uint64_t cycle_length;
      uint8_t max_cap;
      string ipfsimage;
      string details;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("gotchabadge"), gotchabadge> gotchabadge_table;

    // scoped by org
    TABLE gotchaclaim {
      name account;
      name badge;
      uint8_t balance;
      time_point last_claimed_time;
      auto primary_key() const {return account.value; }
    };
    typedef multi_index<name("gotchaclaim"), gotchaclaim> gotchaclaim_table;



    // scoped by org
    TABLE rollupbadge {
      name badge;
      vector<badge_count> rollup_criteria;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("rollupbadge"), rollupbadge> rollupbadge_table;


    // scoped by org
    TABLE achievements {
      name account;
      name badge;
      uint32_t count;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("achievements"), achievements> achievements_table;


};
