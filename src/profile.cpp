#include <profile.hpp>


  ACTION profile::createsimple (name org, name badge, vector<name> parentbadge, string ipfsimage, string details) {
    require_auth(org);
  }

  ACTION profile::creategotcha (name org, name badge, time_point starttime, uint64_t cycle_length, uint8_t max_cap, string ipfsimage, string details) {
    require_auth(org);
  }

  ACTION profile::claimgotcha (name org, name account) {
    require_auth(org);
    check(false, "test error");
  }

  ACTION profile::givegotcha (name org, name from, name to, uint8_t amount, string memo ) {
    require_auth(org);
  }

  ACTION profile::givesimple (name org, name to, name badge, string memo ) {
    require_auth(org);
  }

  ACTION profile::rollup (name org, name account, name badge, vector<badge_count> existing_badges) {
    require_auth(org);
  }





