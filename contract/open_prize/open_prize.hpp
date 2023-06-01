#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include <string>

using namespace eosio;
using namespace std;

class open_prize : public contract { 

public:
    open_prize(account_name self) : contract(self),
    counters(_self, _self){ }

  
    // @abi action
    void initcter();
    // @abi action
    void erasea();
    // @abi action
    void openp();   
    // @abi action
    void modhour(uint64_t daytime);
    // @abi action
    void modday(uint64_t daytime);


    void transfer(account_name from, account_name to, asset quantity, std::string memo);

private:   
  
    // constants
    const static uint64_t ONE_HOUR = 60 * 60* 1;
    const static uint64_t ONE_DAY = 60 * 60 * 24;
    const static uint64_t START_TIME = 1541865600;

	// @abi table counters i64
	struct counter{
		uint64_t		id;
	    uint64_t  hour_time; 
        uint64_t  day_time;        
                                   
		uint64_t		primary_key() const { return id; }

		EOSLIB_SERIALIZE(counter, (id)(hour_time)(day_time));
	};

	typedef eosio::multi_index< N(counters), counter> counters_index;
    counters_index	counters;    

};
