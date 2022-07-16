#include <profile.hpp>
#include <queue>


  ACTION profile::createsimple (name org, name badge, vector<name> parentbadge, string ipfsimage, string details) {
    require_auth(org);

    simplebadge_table _simplebadge (_self, org.value);
    auto badge_itr = _simplebadge.find(badge.value);

    check(badge_itr == _simplebadge.end(), "badge already exists");
    for(auto i = 0; i < parentbadge.size(); i++) { 
      auto parentbadge_itr = _simplebadge.require_find(parentbadge[i].value, "parent badge not found");
    }
    // todo check cycle
    // todo add in all badges
    _simplebadge.emplace(org, [&](auto& row) {
      row.badge = badge;
      row.parentbadge = parentbadge;
      row.ipfsimage = ipfsimage;
      row.details = details;
    });
    addbadge(org, badge, name("simple"));
  }

  ACTION profile::creategotcha (name org, name badge, time_point_sec starttime, uint64_t cycle_length, uint8_t max_cap, string ipfsimage, string details) {
    require_auth(org);

    gotchabadge_table _gotchabadge (_self, org.value);
    
    // todo add in all badges
    auto badge_itr = _gotchabadge.find(badge.value);
    check(badge_itr == _gotchabadge.end(), "badge already exists");

    _gotchabadge.emplace(org, [&](auto& row) {
      row.badge = badge;
      row.starttime = starttime;
      row.cycle_length = cycle_length;
      row.last_known_cycle_start = starttime;
      row.last_known_cycle_end = starttime + cycle_length - 1;
      row.max_cap = max_cap;
      row.ipfsimage = ipfsimage;
      row.details = details;
    });
    addbadge(org, badge, name("gotcha"));
  }

  ACTION profile::createrollup (name org, name badge, vector<badge_count> rollup_criteria, string ipfsimage, string details)  {
    require_auth(org);

    rollupbadge_table _rollupbadge (_self, org.value);
    auto badge_itr = _rollupbadge.find (badge.value);
    check(badge_itr == _rollupbadge.end(), "rollup badge already exists");
    _rollupbadge.emplace(org, [&](auto& row) { 
      row.badge = badge;
      row.rollup_criteria = rollup_criteria;
      row.ipfsimage = ipfsimage;
      row.details = details;
    });
    addbadge(org, badge, name("rollup"));

  }
  ACTION profile::givegotcha (name org, name badge, name from, name to, uint8_t amount, string memo ) {
    require_auth(org);

    require_recipient(from);
    require_recipient(to);

    gotchabadge_table _gotchabadge (_self, org.value);
    auto badge_itr = _gotchabadge.require_find(badge.value, "Not a valid gotcha badge");
    time_point_sec last_known_cycle_start = badge_itr->last_known_cycle_start;
    time_point_sec last_known_cycle_end = badge_itr->last_known_cycle_end;
    uint64_t cycle_length = badge_itr->cycle_length;
    uint8_t max_cap = badge_itr->max_cap;

    time_point_sec current_time = time_point_sec(current_time_point());
    check(current_time >= last_known_cycle_start, "can not give this badge yet");

    bool current_cycle_found = last_known_cycle_start <= current_time && last_known_cycle_end >= current_time;
    bool cycle_update_needed = false;

    while(!current_cycle_found) {
      cycle_update_needed = true; // optimize this.
      last_known_cycle_start = last_known_cycle_start + cycle_length;
      last_known_cycle_end = last_known_cycle_end + cycle_length;
      current_cycle_found = last_known_cycle_start <= current_time && last_known_cycle_end >= current_time;
    }

    time_point_sec current_cycle_start = last_known_cycle_start;
    time_point_sec current_cycle_end = last_known_cycle_end;

    if(cycle_update_needed) {
      _gotchabadge.modify(badge_itr, org, [&](auto& row) {
        row.last_known_cycle_end = current_cycle_end;
        row.last_known_cycle_start = current_cycle_start;
      }); 
    } 


    gotchastats_table _gotchastats (_self, org.value);
    auto account_badge_index = _gotchastats.get_index<name("accountbadge")>();
    uint128_t account_badge_key = ((uint128_t) from.value) << 64 | badge.value;
    auto account_badge_iterator = account_badge_index.find (account_badge_key);

    if(account_badge_iterator == account_badge_index.end()) {
      _gotchastats.emplace(org, [&](auto& row) {
        row.id = _gotchastats.available_primary_key();
        row.account = from;
        row.badge = badge;
        row.balance = amount;
        row.last_claimed_time = current_time;
      });
    } else if (current_cycle_start <= account_badge_iterator->last_claimed_time) {
      check(account_badge_iterator->balance + amount <= max_cap, "Overdrawn balance");
      account_badge_index.modify(account_badge_iterator, org, [&](auto& row) {
        row.balance = row.balance + amount;
        row.last_claimed_time = current_time;
      });
    } else {
      account_badge_index.modify(account_badge_iterator, org, [&](auto& row) {
        row.balance = amount;
        row.last_claimed_time = current_time;
      });
    }

    addachievement(org, badge, to, amount);

  }

  ACTION profile::givesimple (name org, name to, name badge, string memo ) {
    require_auth(org);

    require_recipient(to);

    simplebadge_table _simplebadge (_self, org.value);
    auto badge_itr = _simplebadge.require_find(badge.value, "invalid badge");
    vector<name> all_badges;
    queue<name> _helper_queue;

    all_badges.push_back(badge);
    for(auto i = 0; i < badge_itr->parentbadge.size(); i++) {
      _helper_queue.push(badge_itr->parentbadge[i]);
    }

    while(!_helper_queue.empty()) {
      auto parent_itr = _simplebadge.require_find(_helper_queue.front().value, "invalid parent badge");
      all_badges.push_back(_helper_queue.front()); 
      _helper_queue.pop();
      for(auto i = 0; i < parent_itr->parentbadge.size(); i++) {
          _helper_queue.push(parent_itr->parentbadge[i]);
      }
    }

    for (auto i = 0 ; i < all_badges.size() ; i++ ) {
      addachievement(org, all_badges[i], to, 1);
    }
    
  }

  ACTION profile::takerollup (name org, name account, name badge) {
    require_auth(org);

    rollupbadge_table _rollupbadge (_self, org.value);
    auto badge_itr = _rollupbadge.require_find (badge.value, "no such rollup badge present");
    vector<badge_count> rollup_criteria = badge_itr->rollup_criteria;

    achievements_table _achievements( _self, org.value );
    auto account_badge_index = _achievements.get_index<name("accountbadge")>();
    uint128_t account_badge_key = ((uint128_t) account.value) << 64 | badge.value;
    auto account_badge_iterator = account_badge_index.find (account_badge_key);

    check(account_badge_iterator == account_badge_index.end(), "Already claimed");

    for(auto i = 0 ; i < rollup_criteria.size() ; i++) {
      auto idx = _achievements.get_index<name("accountbadge")>();
      uint128_t key = ((uint128_t) account.value) << 64 | rollup_criteria[i].badge.value;
      auto itr = idx.find (key);
      check(itr != idx.end(), "badge not found, criteria unmet");
      check(itr->count >= rollup_criteria[i].count, "count of badge less, criteria unmet");  
    }
    addachievement(org, badge, account, 1);
  }

// todos 
// improved error messages.
// give gotcha to urself?
// is_account.
// achievement table structure.


