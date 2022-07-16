#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

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
  ACTION creategotcha (name org, name badge, time_point_sec starttime, uint64_t cycle_length, uint8_t max_cap, string ipfsimage, string details);

  /*
  * Claim gotcha badges for current cycle.
  *
  *
  * @param org to which this badge belong. e.g. EdenCN, EdenEN.
  * @param account is claimer.
  * @return no return value.
  */

  ACTION createrollup (name org, name badge, vector<badge_count> rollup_criteria, string ipfsimage, string details);


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
  ACTION givegotcha (name org, name badge, name from, name to, uint8_t amount, string memo );

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
  ACTION takerollup (name org, name account, name badge);

  private:
    // scoped by org
    TABLE allbadges {
      name badge;
      name kind;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("allbadges"), allbadges> allbadges_table;
    
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
      time_point_sec starttime; //0000
      uint64_t cycle_length; // 24 hrs
      time_point_sec last_known_cycle_start;
      time_point_sec last_known_cycle_end;
      uint8_t max_cap;
      string ipfsimage;
      string details;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("gotchabadge"), gotchabadge> gotchabadge_table;

    // scoped by org
    TABLE gotchastats {
      uint64_t id;
      name account;
      name badge;
      uint8_t balance;
      time_point last_claimed_time;
      auto primary_key() const {return id; }
      uint128_t acc_badge_key() const {
        return ((uint128_t) account.value) << 64 | badge.value;
      }
    };
    typedef multi_index<name("gotchastats"), gotchastats,
    indexed_by<name("accountbadge"), const_mem_fun<gotchastats, uint128_t, &gotchastats::acc_badge_key>>
    > gotchastats_table;



    // scoped by org
    TABLE rollupbadge {
      name badge;
      vector<badge_count> rollup_criteria;
      string ipfsimage;
      string details;
      auto primary_key() const {return badge.value; }
    };
    typedef multi_index<name("rollupbadge"), rollupbadge> rollupbadge_table;


    // scoped by org
    TABLE achievements {
      uint64_t id;
      name account;
      name badge;
      uint32_t count;
      auto primary_key() const {return id; }
      uint128_t acc_badge_key() const {
        return ((uint128_t) account.value) << 64 | badge.value;
      }
    };
    typedef multi_index<name("achievements"), achievements,
    indexed_by<name("accountbadge"), const_mem_fun<achievements, uint128_t, &achievements::acc_badge_key>>
    > achievements_table;

    void addbadge(name org, name badge, name kind) {
      allbadges_table _allbadges( _self, org.value );
      auto itr = _allbadges.find(badge.value);
      check(itr == _allbadges.end(), "badge name already taken");
      _allbadges.emplace(org, [&](auto& row){
        row.badge = badge;
        row.kind = kind;
      });
    }

    void addachievement(name org, name badge, name account, uint8_t count) {
      achievements_table _achievements( _self, org.value );
      auto account_badge_index = _achievements.get_index<name("accountbadge")>();
      uint128_t account_badge_key = ((uint128_t) account.value) << 64 | badge.value;
      auto account_badge_iterator = account_badge_index.find (account_badge_key);

      if(account_badge_iterator == account_badge_index.end()) {
        _achievements.emplace(org, [&](auto& row){
          row.id = _achievements.available_primary_key();
          row.account = account;
          row.badge = badge;
          row.count = count;
        });
      } else {
        account_badge_index.modify(account_badge_iterator, org, [&](auto& row){
          row.count = row.count + count;
        });
      }
    }
};
