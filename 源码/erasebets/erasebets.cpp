#include <string>
#include <vector>
#include <eosiolib/transaction.hpp>
#include "erasebets.hpp"

#define EOS_SYMBOL S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)
#define HORSE_CONTRACT N(eosyxsmcoin1)

void erasebets::initcter() {                         //init table,counters
    require_auth(_self);

    counters.emplace(_self, [&](auto& g){
				g.id = 0;
				g.erase_time = START_TIME+INTERVAL;                   
		});
  }

void erasebets::erasea() {
     if( !has_auth(_self) && !has_auth(N(latelatelate)) ) {
          eosio_assert(false,"wrong");
     }
     for (auto clean = counters.begin(); clean != counters.end();){         // erase table,counters
     clean = counters.erase(clean);
     } 
  }

//防止action失败，实现外部转账初始化，并再次触发循环
void erasebets::transfer(account_name from, account_name to, asset quantity, std::string memo) {
   
    if (from == _self || to != _self|| from ==N(eosio.ram)||from ==N(eosio.stake)) {return;  }   
 
    eosio_assert(from == N(latelatelate),"must from latelatelate");
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount == 18, " must be  0.0018 EOS");        //must be 0.0018 EOS 
    eosio_assert(quantity.symbol == EOS_SYMBOL, "asset must be EOS");    
    erasea();
    counters.emplace(_self, [&](auto& g){
				g.id = 0;
				g.erase_time = now() + INTERVAL;                    //必须整点进行转账，不然会影响时间           
		});

    action(
        permission_level{_self, N(active)},
        _self, N(openp),
        make_tuple()).send();
}

void erasebets::openp() {
   
    require_auth(_self);

    auto itr = counters.find(0);    
    eosio_assert(itr != counters.end(), "hour,counters id  0 does not exist");    


     if(now() >= itr->erase_time){
      
      action(
        permission_level{_self, N(active)},
        HORSE_CONTRACT, N(erasebets),
        make_tuple()).send();      

      counters.modify(itr, _self, [&](auto &gl) { gl.erase_time += INTERVAL; });
    }



     //循环，每5分钟自触发一次
    transaction trx;
    trx.actions.emplace_back(permission_level{_self, N(active)},
                _self,N(openp),
                make_tuple()
    );
    trx.delay_sec = 600;
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
EOSIO_ABI(erasebets,(transfer)(openp)(erasea)(initcter))
