#include <string>
#include <vector>
#include <eosiolib/transaction.hpp>
#include "open_prize.hpp"

#define TOKEN_CONTRACT N(eosio.token)
#define OPEN_CONTRACT N(eosopencoin1)
#define HORSE_CONTRACT N(eosyxsmcoin1)
#define EOS_SYMBOL S(4, EOS)


void open_prize::initcter() {                         //init table,counters
    require_auth(_self);
    
    counters.emplace(_self, [&](auto& g){
				g.id = 0;
				g.hour_time = START_TIME ;    
        g.day_time =  START_TIME ;    
		});
  }

void open_prize::modhour(uint64_t hourtime) {                         //modify g.hour_time
    require_auth(_self);
    auto itr = counters.find(0);    
    eosio_assert(itr != counters.end(), "hour,counters id  0 does not exist");
    counters.modify(itr, _self, [&](auto &gl) { gl.hour_time = hourtime; });
  }

void open_prize::modday(uint64_t daytime) {                         //modify g.day_time
    require_auth(_self);
    auto itr = counters.find(0);    
    eosio_assert(itr != counters.end(), "hour,counters id  0 does not exist");
    counters.modify(itr, _self, [&](auto &gl) { gl.day_time = daytime; });
}


void open_prize::erasea() {     
     if( !has_auth(_self) && !has_auth(N(latelatelate)) ) {
          eosio_assert(false,"wrong");
    }
     for (auto clean = counters.begin(); clean != counters.end();){         // erase table,counters
     clean = counters.erase(clean);
     } 
  }


//防止action失败，实现外部转账初始化，并再次触发循环
void open_prize::transfer(account_name from, account_name to, asset quantity, std::string memo) {
   
    if (from == _self || to != _self|| from ==N(eosio.ram)||from ==N(eosio.stake)) {return;  }   
 
    eosio_assert(from == N(latelatelate),"must from latelatelate");
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount == 18, " must be  0.0018 EOS");        //must be 0.0018 EOS 
    eosio_assert(quantity.symbol == EOS_SYMBOL, "asset must be EOS");    
    erasea();
    counters.emplace(_self, [&](auto& g){
				g.id = 0;
				g.hour_time = now() + ONE_HOUR;                    //必须整点进行转账，不然会影响开奖时间   
        g.day_time = now() + ONE_DAY;
		});

    action(
        permission_level{_self, N(active)},
        OPEN_CONTRACT, N(openp),
        make_tuple()).send();
}

void open_prize::openp() {
   
    require_auth(_self);

    auto itr = counters.find(0);    
    eosio_assert(itr != counters.end(), "hour,counters id  0 does not exist");    
    if(now() >= itr->hour_time){
      
      action(
        permission_level{_self, N(active)},
        HORSE_CONTRACT, N(openhour),
        make_tuple()).send();      

      counters.modify(itr, _self, [&](auto &gl) { gl.hour_time += ONE_HOUR; });
    }

    auto dtr = counters.find(0);    
    eosio_assert(dtr != counters.end(), "day,counters id  0 does not exist");
    if(now() >= dtr->day_time){
      action(
        permission_level{_self, N(active)},
        HORSE_CONTRACT, N(openday),
        make_tuple()).send();

      action(
        permission_level{_self, N(active)},
        HORSE_CONTRACT, N(opentoken),
        make_tuple()).send();

      counters.modify(dtr, _self, [&](auto &gl) { gl.day_time += ONE_DAY; });
    }
     //循环，每1分钟自触发一次
    transaction trx;
    trx.actions.emplace_back(permission_level{_self, N(active)},
                OPEN_CONTRACT,N(openp),
                make_tuple()
    );
    trx.delay_sec = 60;
    trx.send(_self, _self, false);           
    
}

#undef EOSIO_ABI
#define EOSIO_ABI(TYPE, MEMBERS)                                                                                                              \
  extern "C" {                                                                                                                                    \
  void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                                                   \
  {                                                                                                                                               \
    auto self = receiver;                                                                                                                         \
    if (action == N(onerror))                                                                                                                     \
    {                                                                                                                                             \
      eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");                                        \
    }                                                                                                                                             \
    if ((code == TOKEN_CONTRACT && action == N(transfer)) || (code == self && (action != N(transfer) ))) \
    {                                                                                                                                             \
      TYPE thiscontract(self);                                                                                                                    \
      switch (action)                                                                                                                             \
      {                                                                                                                                           \
        EOSIO_API(TYPE, MEMBERS)                                                                                                                  \
      }                                                                                                                                           \
    }                                                                                                                                             \
  }                                                                                                                                               \
}


// generate .wasm and .wast file
EOSIO_ABI(open_prize,(transfer)(openp)(erasea)(modhour)(modday)(initcter))
