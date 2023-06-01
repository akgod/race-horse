#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include <string>

using namespace eosio;
using namespace std;



class erasebets : public contract { 

public:
    erasebets(account_name self) : contract(self),
    counters(_self, _self){ }

  
    // @abi action
    void initcter();
    // @abi action
    void erasea();
    // @abi action
    void openp();   

    void transfer(account_name from, account_name to, asset quantity, std::string memo);
   

private:   
    // constants
    const static uint64_t INTERVAL = 60 * 60* 6;
    const static uint64_t START_TIME = 1541592000;


	// @abi table counters i64
	struct counter{
		uint64_t		id;
	    uint64_t  erase_time;                                                 
		uint64_t		primary_key() const { return id; }
		EOSLIB_SERIALIZE(counter, (id)(erase_time));
	};

	typedef eosio::multi_index< N(counters), counter> counters_index;
    counters_index	counters;    
};
