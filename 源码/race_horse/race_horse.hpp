#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/contract.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/time.hpp>
#include <string>

using namespace eosio;
using namespace std;
using eosio::time_point_sec;

class race_horse : public contract { 

public:
    race_horse(account_name self) : contract(self),
    globalvars(_self, _self),
    airdrops(_self, _self),
    dayinviters(_self, _self),
    ivtranks(_self,_self),
    players(_self, _self),
    activebets(_self, _self),
    daytokenpls(_self, _self),
    hourplayers(_self, _self),
    hourtens(_self, _self),
    dayplayers(_self, _self),
    dayfiftys(_self, _self),
    hourprizes(_self, _self),
    dayprizes(_self, _self){ }

  
    // @abi action
    void initglo();
    // @abi action
    void initair();
    // @abi action
    void erasetab(uint64_t tab);   
    // @abi action
    void erasebets();
    // @abi action
    void openhour();
    // @abi action
    void openday();
    // @abi action
    void opentoken();
    
    void transfer(account_name from, account_name to, asset quantity, std::string memo);
    void withdraw(const account_name account, asset quantity);
             
   

private: 
    // constants
    const static uint64_t HOUR_TIMES = 10;
    const static uint64_t DAY_TIMES = 50;
    const static uint64_t MIN_BET = 1000;            //min bet 0.1 EOS
    const static uint64_t MAX_BET = 2000000;          //max bet 200 EOS    
    constexpr static double DIV_BONUS = 0.0025;           //2% to bonus_div, for the hour-hour_bonus and day-day_bonus
    constexpr static double INV_BONUS = 0.005;         //0.5% to bonus_inv,for the inviter
    constexpr static double DIV_PROFIT = 0.25;           //25% to token_bonus pool


	// @abi table globalvars i64
	struct globalvar{
		uint64_t		id;
	    uint64_t hour_bonus ;                  //bet-amout 0.5%,this hour. to be 0, everyhour

        uint64_t day_bonus ;                   //bet-amout 0.5%,today.     to be 0, everyday

        uint64_t day_profit ;                  //the amout of eos the team get.  to be 0,everyday
        uint64_t day_amount ;                  //the amout of eos palyers bet, today .to be 0,everyday
        uint64_t day_token  ;                  //the amout of token,which user can get the profit .to be 0,everyday

        uint64_t hourprize_id;                    //the table hourprizes   id
        uint64_t dayprize_id;                    //the table  dayprizes   id	
        uint64_t ivtrank_id;                     //the table ivtranks  id

		uint64_t		primary_key() const { return id; }

		EOSLIB_SERIALIZE(globalvar, (id)(hour_bonus)(day_bonus)(day_profit)(day_amount)(day_token)(hourprize_id)(dayprize_id)(ivtrank_id));
	};

	typedef eosio::multi_index< N(globalvars), globalvar> globalvars_index;
    globalvars_index	globalvars;

	// @abi table airdrop i64
	struct airdrop{
		uint64_t		id;
	    uint64_t        new_tok ;                  //new player,airdrop

        uint64_t        mining_tok ;                   //airdrop
		uint64_t		primary_key() const { return id; }

		EOSLIB_SERIALIZE(airdrop, (id)(new_tok)(mining_tok));
	};

	typedef eosio::multi_index< N(airdrop), airdrop> airdrop_index;
    airdrop_index	airdrops;



    // @abi table player i64
    struct playeritem {
        account_name account;
        account_name inviter;
        uint64_t myivttimes;
        uint64_t tokenbonus;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(playeritem, (account)(inviter)(myivttimes)(tokenbonus))
    };
    typedef eosio::multi_index<N(player), playeritem> player_index;
    player_index players;


    // @abi table daytokenpl i64
    struct daytplsitem {
        account_name account;
        uint64_t token;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(daytplsitem, (account)(token))
    };
    typedef eosio::multi_index<N(daytokenpl), daytplsitem> daytokenpls_index;
    daytokenpls_index daytokenpls;

    // @abi table hourplayer i64
    struct hourplitem {
        account_name account;
        uint64_t times;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(hourplitem, (account)(times))
    };
    typedef eosio::multi_index<N(hourplayer), hourplitem> hourplayer_index;
    hourplayer_index hourplayers;

    // @abi table hourten i64
    struct hourtenitem {
        account_name account;
        uint64_t times;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(hourtenitem, (account)(times))
    };
    typedef eosio::multi_index<N(hourten), hourtenitem> hourten_index;
    hourten_index hourtens;


    // @abi table dayplayer i64
    struct dayplitem {
        account_name account;
        uint64_t times;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(dayplitem, (account)(times))
    };
    typedef eosio::multi_index<N(dayplayer), dayplitem> dayplayer_index;
    dayplayer_index dayplayers;


    // @abi table dayfifty i64
    struct dayfifitem {
        account_name account;
        uint64_t times;
        uint64_t primary_key() const { return account; }
        EOSLIB_SERIALIZE(dayfifitem, (account)(times))
    };
    typedef eosio::multi_index<N(dayfifty), dayfifitem> dayfifty_index;
    dayfifty_index dayfiftys;



    // @abi table hourprize i64
    struct hourpritem {
        uint64_t 		id;
        account_name account;
        uint64_t prizeamt;
        uint64_t time;
        uint64_t primary_key() const { return id; }
        uint64_t get_time()   const {return time; }
        EOSLIB_SERIALIZE(hourpritem, (id)(account)(prizeamt)(time))
    };
    typedef eosio::multi_index<N(hourprize), hourpritem,
                    indexed_by< N(time), const_mem_fun<hourpritem, uint64_t, &hourpritem::get_time>>> hourprize_index;
    hourprize_index hourprizes;
    
    // @abi table dayprize i64
    struct daypritem {
        uint64_t 		id;
        account_name account;
        uint64_t prizeamt;
        uint64_t time;
        uint64_t primary_key() const { return id; }
        uint64_t get_time()   const {return time; }
        EOSLIB_SERIALIZE(daypritem, (id)(account)(prizeamt)(time))
    };
    typedef eosio::multi_index<N(dayprize), daypritem,
                  indexed_by< N(time), const_mem_fun<daypritem, uint64_t, &daypritem::get_time>>> dayprize_index;
    dayprize_index dayprizes;



    // @abi table dayinvite i64
    struct dayinvite {
        account_name account;
        uint64_t	quantity;
        uint64_t primary_key() const { return account; }
        uint64_t get_quantity() const { return quantity; }
        EOSLIB_SERIALIZE(dayinvite, (account)(quantity))
    };
    typedef eosio::multi_index<N(dayinvite), dayinvite,
            indexed_by< N(quantity), const_mem_fun<dayinvite, uint64_t, &dayinvite::get_quantity>>> dayinvite_index;

    dayinvite_index dayinviters;


    // @abi table ivtrank i64
    struct ivtrank {
        uint64_t 		id;
        account_name account;
        uint64_t	quantity;
        uint64_t     time;
        uint64_t primary_key() const { return id; }
        uint64_t get_time()   const {return time; }
        EOSLIB_SERIALIZE(ivtrank, (id)(account)(quantity)(time))
    };
    typedef eosio::multi_index<N(ivtrank), ivtrank,
             indexed_by< N(time), const_mem_fun<ivtrank, uint64_t, &ivtrank::get_time>>> ivtrank_index;

    ivtrank_index ivtranks;
    

	// @abi table activebets i64
	struct bet {
		uint64_t 		id;
		account_name	bettor;
		uint64_t		bet_amt;
        uint64_t        odds;
		uint64_t		roll_under;
        uint64_t        random_number;
		uint64_t        bet_time;
        uint64_t        bonus;
        
			
		uint64_t 		primary_key() const { return id; }
		uint64_t          get_time() const {return bet_time;}	   
        account_name     get_bettor() const {return bettor;}
		EOSLIB_SERIALIZE( bet, (id)(bettor)(bet_amt)(odds)(roll_under)(random_number)(bet_time)(bonus))
		};

	typedef eosio::multi_index< N(activebets), bet,
                     indexed_by< N(bet_time), const_mem_fun<bet, uint64_t, &bet::get_time>>,
                     indexed_by< N(bettor), const_mem_fun<bet, uint64_t, &bet::get_bettor>>> bets_index;

	bets_index	activebets;	
   
    struct token
    {
        token(account_name tkn) : _self(tkn) {}
        struct account
        {
            eosio::asset    balance;
            uint64_t primary_key()const { return balance.symbol.name(); }
        };
        typedef eosio::multi_index<N(accounts), account> accounts;

        asset get_balance(account_name owner,  eosio::symbol_type sym) const
        {
            accounts acnt(_self, owner);
            auto itr = acnt.find( sym.name() );
            if (itr != acnt.end()) {
                return acnt.find(sym.name())->balance;
            } else {
                return asset(0, sym);
            }
        }

       private:
          account_name _self;
    };


    void bet(uint64_t bet_id,account_name account,  asset quantity, uint64_t rock_under,uint64_t odds );   


};
