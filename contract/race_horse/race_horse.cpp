#include <string>
#include <vector>
#include <eosiolib/transaction.hpp>
#include "RandomSeed.hpp"
#include "race_horse.hpp"

#define EOS_SYMBOL S(4, EOS)
#define EOSYX_SYMBOL S(4, EOSYX)

#define TOKEN_CONTRACT N(eosio.token)
#define EOSYX_CONTRACT N(eosyouxicoin)
#define OPEN_CONTRACT N(eosopencoin1)
#define ERASE_CONTRACT N(eossssscoin1)

void race_horse::initglo() {                         //init table,globalvars
    globalvars.emplace(_self, [&](auto& g){
				g.id = 0;
				g.hour_bonus = 0;
        g.day_bonus  = 0;
        g.day_profit = 0;
        g.day_amount = 0;
        g.day_token = 0;
        g.hourprize_id = 0;
        g.dayprize_id = 0;
        g.ivtrank_id = 0;
		});
}

void race_horse::initair() {                         //init table,airdrops
    require_auth(_self);	
    airdrops.emplace(_self, [&](auto& t){
				t.id = 0;
				t.new_tok = 0;
        t.mining_tok  = 0;
		});
}


void race_horse::erasetab(uint64_t tab) {
     require_auth(_self);

    uint64_t which_tab = tab;

    if(which_tab == 0){
        for (auto clean = globalvars.begin(); clean != globalvars.end();){         // erase table,globalvars
           clean = globalvars.erase(clean);
         } 
     }
    
    if(which_tab == 1){
        for (auto clean = dayplayers.begin(); clean != dayplayers.end();){         // erase table,dayplayers
          clean = dayplayers.erase(clean);
        } 
     } 
    if(which_tab == 2){
        for (auto clean = dayprizes.begin(); clean != dayprizes.end();){         // erase table,dayprizes
         clean = dayprizes.erase(clean);
        } 
    } 
    if(which_tab == 3){
       
        for(auto clean = activebets.begin(); clean != activebets.end();){           // erase table,activebers.                                                                
                                                            
            clean = activebets.erase(clean);
        } 
    } 
    if(which_tab == 4){
        for (auto clean = hourprizes.begin(); clean != hourprizes.end();){         // erase table,hourprizes
          clean = hourprizes.erase(clean);
        } 
    } 
    if(which_tab == 5){
        for (auto clean = dayfiftys.begin(); clean != dayfiftys.end();){         // erase table,dayfiftys
           clean = dayfiftys.erase(clean);
        }  
    } 

    if(which_tab == 6){
        for (auto clean = daytokenpls.begin(); clean != daytokenpls.end();){         // erase table,daytokenpls
           clean = daytokenpls.erase(clean);
        } 
    } 
    if(which_tab == 7){
         for (auto clean = players.begin(); clean != players.end();){         // erase table,players
            clean = players.erase(clean);
         }  
     } 
    if(which_tab == 8){
          for (auto clean = ivtranks.begin(); clean != ivtranks.end();){         // erase table,ivtranks
            clean = ivtranks.erase(clean);
          } 
    } 
}


void race_horse::erasebets(){
        
        if( !has_auth(_self) && !has_auth(ERASE_CONTRACT) ) {
          eosio_assert(false,"wrong");
        }
        auto clean = activebets.begin();
        for(int i =0;i <200;i++){                  
            if(clean != activebets.end()) {                                // erase table,activebers. 
                clean = activebets.erase(clean);                
            }           
        } 
        if(clean != activebets.end()){
            transaction trx;
            trx.actions.emplace_back(permission_level{_self, N(active)},
                _self,N(erasebets),
                make_tuple()
             );
            trx.delay_sec = 2;
            trx.send(_self, _self, false);
        }
}


void race_horse::withdraw(const account_name account, asset quantity) {

  // find user
  auto itr = players.find(account);
  eosio_assert(itr != players.end(), "player does not exist");

  // set quantity
  eosio_assert(quantity.amount + itr->tokenbonus > quantity.amount,
               "integer overflow adding withdraw tokenbonus");
  quantity.amount += itr->tokenbonus;

  // clear tokenbonus
  players.modify(itr, _self, [&](auto &p) { p.tokenbonus = 0; });

  // withdraw
  action(permission_level{_self, N(active)}, N(eosio.token),
         N(transfer), std::make_tuple(_self, account, quantity,
                                      std::string("eosyx.io 超级分红池提现")))
      .send();
}


void race_horse::transfer(account_name from, account_name to, asset quantity, std::string memo) {
   
    if (from == _self || to != _self || from ==N(eosio.ram)||from ==N(eosio.stake)) {
      return;
    }   
       
    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount >= MIN_BET, "Bet must large than 0.1 EOS");        //min bet 0.1 EOS
    eosio_assert(quantity.amount <= MAX_BET, "Bet must small than 200 EOS");     //max bet 200 EOS
    eosio_assert(quantity.symbol == EOS_SYMBOL, "asset must be EOS");
   

   auto glo = globalvars.find(0);
   eosio_assert(glo != globalvars.end(), "globalvars does not exist");
   
    //when the day_amout is small than 80000,the player can get token_bonus,which is 30% of the day_profit
    if(glo->day_amount < 800000000){
      auto a = daytokenpls.find(from);    
      if(a == daytokenpls.end()){
        a = daytokenpls.emplace(_self, [&](auto& item){
        item.account = from;
        item.token = quantity.amount;
        });
      }
      else{
        daytokenpls.modify(a, _self, [&](auto& item){
				item.token += quantity.amount;
				});  
      }
      globalvars.modify(glo, _self, [&](auto &gl) { gl.day_token = glo->day_token + quantity.amount; });
    }
    globalvars.modify(glo, _self, [&](auto &gl) { gl.day_amount = glo->day_amount + quantity.amount; });

    auto b = hourplayers.find(from);
    if(b == hourplayers.end()){
       b = hourplayers.emplace(_self, [&](auto& item){
      item.account = from;
      item.times = 1;
      });
    }
    else{
      hourplayers.modify(b, _self, [&](auto& item){
			item.times += 1;
			});  
    }
    

    auto c = dayplayers.find(from);
 
    if(c == dayplayers.end()){
       c = dayplayers.emplace(_self, [&](auto& item){
      item.account = from;
      item.times = 1;
      });
    }
    else{
      dayplayers.modify(c, _self, [&](auto& item){
			item.times += 1;
			});  
    }

    

    // probability set ,get inviter

  		std::string rate_str;
			std::string inviter_str;

			const std::size_t first_break = memo.find("-");
			rate_str = memo.substr(0, first_break);

			if(first_break != std::string::npos){
				
				const std::string after_first_break = memo.substr(first_break + 1);
				const std::size_t second_break = after_first_break.find("-");

				if(second_break != std::string::npos){					
					inviter_str = after_first_break.substr(0, second_break);
				}
				else {					
					inviter_str = after_first_break;
				}
			}
			else {
				inviter_str = std::string("");			
			}

			account_name inviter = N(latelatelate);
			const account_name possible_inviter = eosio::string_to_name(inviter_str.c_str());                //get inviter
			
			if (possible_inviter != _self && possible_inviter != from && is_account(possible_inviter)){
				inviter = possible_inviter;
			}

    double rate = 0;
    uint64_t odds = 0; 
    if(rate_str == std::to_string(0)){ rate = 1;  odds = 2;}            //1:2   1
    if(rate_str == std::to_string(1)){ rate= 0.4; odds = 3;}           //1:3   *50%
    if(rate_str == std::to_string(2)){ rate= 0.15; odds = 6;}           //1:6   *20%    
    if(rate_str == std::to_string(8)){ return; }              //EOS-pool deposit ,max 2000EOS every deposit

    // if transfer amount is 0.1001 EOS and rate_str=9 then withdraw
    if (quantity.amount == 1001 && rate_str == std::to_string(9)) {withdraw(from, quantity); return;}

    uint64_t rock_number = 49;                                      


    uint64_t rock_under = (uint64_t)(rate * rock_number);

    //generate bet_id
    auto s = read_transaction(nullptr, 0);
		char *tx = (char *)malloc(s);
		read_transaction(tx, s);
		checksum256 tx_hash;
		sha256(tx, s, &tx_hash);
    const uint64_t bet_id = ((uint64_t)tx_hash.hash[0] << 56) + ((uint64_t)tx_hash.hash[1] << 48) + ((uint64_t)tx_hash.hash[2] << 40) + ((uint64_t)tx_hash.hash[3] << 32) + ((uint64_t)tx_hash.hash[4] << 24) + ((uint64_t)tx_hash.hash[5] << 16) + ((uint64_t)tx_hash.hash[6] << 8) + (uint64_t)tx_hash.hash[7];

  //players---inviterankings,inviter bonus
    asset eosyx_balance = token(EOSYX_CONTRACT).get_balance(_self, EOSYX_SYMBOL);

    auto tok = airdrops.find(0);
    eosio_assert(tok != airdrops.end(), "airdrops does not exist");

    auto p = players.find(from);
    if( p == players.end()){                    //not found,new player

      p = players.emplace(_self, [&](auto& item){
        item.account = from;
        item.inviter = inviter;
        item.myivttimes = 0;
        item.tokenbonus = 0;
      });

       
      auto mts = players.find(inviter);                 
      if( mts != players.end()){                //found,the inviter->myivttimes+1.
		  	players.modify(mts, _self, [&](auto& item){
		  	item.myivttimes += 1;
		  	});    
      }    

    
       auto d = dayinviters.find(inviter);                 //dayinviters
       if( d == dayinviters.end()){                    //not found,new inviter

          d = dayinviters.emplace(_self, [&](auto& item){
          item.account = inviter;
          item.quantity = 1;
          });
       }
        else{                                                    //found,quantity+1
				   dayinviters.modify(d, _self, [&](auto& item){
					 item.quantity += 1;
				   });    

        }

      if (eosyx_balance.amount >= 3000000) {                          //............new player ,airdrop 300 SSS
          if(tok->new_tok < 45000000000){
            uint64_t new_bonus =0;
            if(tok->new_tok < 30000000000 ){ new_bonus =1000000;}
            if(tok->new_tok >=30000000000 ){ new_bonus =500000;}
        
            action(
              permission_level{_self, N(active)},
              EOSYX_CONTRACT, N(transfer),
              make_tuple(_self, from, asset(new_bonus, EOSYX_SYMBOL), std::string("eosyx.io新用户免费空投EOSYX")))
             .send();
               
            airdrops.modify(tok, _self, [&](auto &to) { 
                to.new_tok = to.new_tok + new_bonus;       
             });
          }
      }
    }

    //............old player,1 EOS =1 SSS   
    uint64_t eosyx_back = quantity.amount;
    if (eosyx_back > 0 && eosyx_balance.amount >= eosyx_back) {
        if(tok->mining_tok < 200000000000){
            if(tok->mining_tok > 150000000000 ){
                eosyx_back =uint64_t(0.5*quantity.amount);
            }

            action(
               permission_level{_self, N(active)},
               EOSYX_CONTRACT, N(transfer),
               make_tuple(_self, from, asset(eosyx_back, EOSYX_SYMBOL), std::string("下注即挖矿 eosyx.io挖矿所得")))
               .send();

            airdrops.modify(tok, _self, [&](auto &to) { 
                to.mining_tok = to.mining_tok + eosyx_back;       
             });
        }
    }          
   bet(bet_id, from, quantity, rock_under, odds);    
}


void race_horse::bet(uint64_t bet_id,account_name account, asset quantity, uint64_t rock_under ,uint64_t odds ) {

    require_auth(account);

    eosio_assert(quantity.amount > 0, "must bet a positive amount");
    eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS allowed" );
    
    uint64_t rock_under_number = rock_under;
    uint64_t odd = odds;
    uint64_t bonus_div = DIV_BONUS * quantity.amount;                        //0.25% to bonus_div, for the hour-hour_bonus and day-day_bonus
    uint64_t bonus_inv = INV_BONUS * quantity.amount;                       //0.5% to bonus_inv,for the inviter
    uint64_t bonus = odd*quantity.amount - bonus_div - bonus_inv;                 //the amount of eos the player win

    auto glo = globalvars.find(0);
    eosio_assert(glo != globalvars.end(), "globalvars does not exist");
    globalvars.modify(glo, _self, [&](auto &gl) { 
       gl.hour_bonus = gl.hour_bonus + bonus_div;
       gl.day_bonus  = gl.day_bonus + bonus_div;
    });
    
    
    uint64_t random_number = (uint64_t) random_gen::get_instance(account,bet_id).range(100);     //random number 0-100 

   //send bonus_inv to the inviter no matter win or lose
   auto q = players.find(account);
   if( q != players.end()){                   
     
      account_name inviteman = q->inviter;    
        
     action(
       permission_level{_self, N(active)},
       TOKEN_CONTRACT, N(transfer),
       make_tuple(_self, inviteman, asset(bonus_inv, EOS_SYMBOL), std::string("邀请奖励 eosyx.io")))
      .send();

    } 
    //open bet
    if(random_number <= rock_under_number){             //player win                

        transaction trx;
        trx.actions.emplace_back(permission_level{_self, N(active)},
                TOKEN_CONTRACT,N(transfer),
                make_tuple(_self, account, asset(bonus, EOS_SYMBOL), std::string("eosyx.io 恭喜您!"))
        );
        trx.delay_sec = 1;
        trx.send(account, _self, false);



        
        int s = (int)glo->day_profit - (int)quantity.amount;           // in case Negative spillover
        if(s < 0 ){                               
            globalvars.modify(glo, _self, [&](auto &gl) { gl.day_profit = 0; });
        }
        else{
            globalvars.modify(glo, _self, [&](auto &gl) { gl.day_profit = gl.day_profit - quantity.amount; });
        }

        //save to table activebets,win
        activebets.emplace(_self, [&](auto& bet){
		      	bet.id = bet_id;
			      bet.bettor = account;	
			      bet.bet_amt = (uint64_t)quantity.amount;
            bet.odds  = odd;
			      bet.roll_under = rock_under_number;
            bet.random_number = random_number;
		      	bet.bet_time = now();
            bet.bonus = bonus;
 	      });
    }
    else{                                             //player lose        
        globalvars.modify(glo, _self, [&](auto &gl) { gl.day_profit = gl.day_profit + quantity.amount; });
       //save to table activebets
        activebets.emplace(_self, [&](auto& bet){
		    	bet.id = bet_id;
		    	bet.bettor = account;	
		    	bet.bet_amt = (uint64_t)quantity.amount;
          bet.odds  = odd;
		    	bet.roll_under = rock_under_number;
          bet.random_number = random_number;
		    	bet.bet_time = now();
        //  bet.bonus = bonus;
 	       });        
    }
}

//open and send hour-hour-prize 
void race_horse::openhour(){  
  require_auth(OPEN_CONTRACT);
  auto glo = globalvars.find(0);  
  eosio_assert(glo != globalvars.end(), "globalvars does not exist");
  uint64_t hour_prize = uint64_t(glo->hour_bonus/5);
  if (hour_prize == 0){hour_prize = 1;}
   
  for (auto itr = hourplayers.begin(); itr != hourplayers.end(); itr++) {            //select the player,  above 10 times,one hour
   
    if(itr->times >= HOUR_TIMES){
      hourtens.emplace(_self, [&](auto& item){
        item.account = itr->account;
        item.times = itr->times;
      }); 
    }     
  }
  uint64_t pls_num = 0; 
  for (auto itr = hourtens.begin(); itr != hourtens.end(); itr++) {   
      pls_num += 1;        
  }
  
  if( pls_num <5 ){ 
    for (auto clear = hourtens.begin(); clear != hourtens.end();){         // erase table,hourtens
     clear = hourtens.erase(clear);
    }    
    return; 
  }  

  uint64_t i =0;
  while( i<5 ) {            

    //generate block seed
    auto s = read_transaction(nullptr, 0);
		char *tx = (char *)malloc(s);
		read_transaction(tx, s);
		checksum256 tx_hash;
		sha256(tx, s, &tx_hash);
    const uint64_t block_seed = ((uint64_t)tx_hash.hash[0] << 56) + ((uint64_t)tx_hash.hash[1] << 48) + ((uint64_t)tx_hash.hash[2] << 40) + ((uint64_t)tx_hash.hash[3] << 32) + ((uint64_t)tx_hash.hash[4] << 24) + ((uint64_t)tx_hash.hash[5] << 16) + ((uint64_t)tx_hash.hash[6] << 8) + (uint64_t)tx_hash.hash[7];
      
      uint64_t luck_number = (uint64_t) random_gen::get_instance(OPEN_CONTRACT,block_seed).range(pls_num);     //random number 0-pls_num
      auto lpl = hourtens.begin();
      uint64_t j =0;
      while(j<luck_number){
         lpl=lpl++;
         j+=1;
      }

      account_name luck_player = lpl->account;      
      

      action(
          permission_level{_self, N(active)},
          TOKEN_CONTRACT, N(transfer),
          make_tuple(_self, luck_player, asset(hour_prize, EOS_SYMBOL), std::string("eosyx.io 时时奖")))
        .send();

     
      //save to table hourprizes
      hourprizes.emplace(_self, [&](auto& item){
        item.id = glo->hourprize_id;
		  	item.account = luck_player;
			  item.prizeamt = hour_prize;	
			  item.time = now();      
 	    });

      //erase the table hourplayers->luck_player,in case of duplicate transaction
      auto cln = hourtens.find(luck_player);      
      cln = hourtens.erase(cln);
      pls_num -=1;

      i+=1;      
      globalvars.modify(glo, _self, [&](auto &gl) { gl.hourprize_id += 1; });
      
  } 
 
  globalvars.modify(glo, _self, [&](auto &gl) { gl.hour_bonus = 0; });

  for (auto clean = hourplayers.begin(); clean != hourplayers.end();){         // erase table,hourplayers
     clean = hourplayers.erase(clean);
  }    

  for (auto clear = hourtens.begin(); clear != hourtens.end();){         // erase table,hourtens
     clear = hourtens.erase(clear);
  } 

}

//open and send day-day-prize
void race_horse::openday(){
  require_auth(OPEN_CONTRACT);
  auto glo = globalvars.find(0);
  eosio_assert(glo != globalvars.end(), "globalvars does not exist");
  uint64_t day_prize = uint64_t(glo->day_bonus/5);
  if(day_prize ==0 ){day_prize = 1;}

  for (auto itr = dayplayers.begin(); itr != dayplayers.end(); itr++) {            //select the player,  above 50 times,one day
   
    if(itr->times >= DAY_TIMES){
      dayfiftys.emplace(_self, [&](auto& item){
        item.account = itr->account;
        item.times = itr->times;
      }); 
    }     
  }
  uint64_t pls_num = 0; 
  for (auto itr = dayfiftys.begin(); itr != dayfiftys.end(); itr++) {   

      pls_num += 1;        
  }  
  if( pls_num <5 ){ 
    for (auto clear = dayfiftys.begin(); clear != dayfiftys.end();){         // erase table,dayfiftys
     clear = dayfiftys.erase(clear);
    }         
    return; 
  }  
  uint64_t i =0;
  while( i<5 ) {
    
      //generate block seed
      auto s = read_transaction(nullptr, 0);
		  char *tx = (char *)malloc(s);
		  read_transaction(tx, s);
		  checksum256 tx_hash;
		  sha256(tx, s, &tx_hash);
      const uint64_t block_seed = ((uint64_t)tx_hash.hash[0] << 56) + ((uint64_t)tx_hash.hash[1] << 48) + ((uint64_t)tx_hash.hash[2] << 40) + ((uint64_t)tx_hash.hash[3] << 32) + ((uint64_t)tx_hash.hash[4] << 24) + ((uint64_t)tx_hash.hash[5] << 16) + ((uint64_t)tx_hash.hash[6] << 8) + (uint64_t)tx_hash.hash[7];

      uint64_t luck_number = (uint64_t) random_gen::get_instance(OPEN_CONTRACT,block_seed).range(pls_num);     //random number 0-pls_num
      auto lpl = dayfiftys.begin();
      uint64_t j =0;
      while(j<luck_number){
         lpl=lpl++;
         j+=1;
      }
      account_name luck_player = lpl->account;         

      action(
          permission_level{_self, N(active)},
          TOKEN_CONTRACT, N(transfer),
          make_tuple(_self, luck_player, asset(day_prize, EOS_SYMBOL), std::string("eosyx.io 天天BOSS奖")))
        .send();

        
      //save to table dayprizes
        dayprizes.emplace(_self, [&](auto& item){
          item.id = glo->dayprize_id;
		  	  item.account = luck_player;
			    item.prizeamt = day_prize;	
			    item.time = now();
 	      });
      //erase the table dayfiftys->luck_player,in case of duplicate transaction
      auto cln = dayfiftys.find(luck_player);
      cln = dayfiftys.erase(cln);

      pls_num -=1;

      i+=1; 
      globalvars.modify(glo, _self, [&](auto &gl) { gl.dayprize_id += 1; });
     
  } 
   
  globalvars.modify(glo, _self, [&](auto &gl) { gl.day_bonus = 0; });

  for (auto clean = dayplayers.begin(); clean != dayplayers.end();){         // erase table,dayplayers
     clean = dayplayers.erase(clean);
  }    
  for (auto clear = dayfiftys.begin(); clear != dayfiftys.end();){         // erase table,dayfiftys
     clear = dayfiftys.erase(clear);
  } 
 
}

//open token bonus, save the data of invite ranks to ivtranks
void race_horse::opentoken(){
  require_auth(OPEN_CONTRACT);
  auto glo = globalvars.find(0);
  eosio_assert(glo != globalvars.end(), "globalvars does not exist");
  uint64_t daytoken_prize = 0 ;
  if (glo->day_amount < 1000000000){
    daytoken_prize  = uint64_t (DIV_PROFIT*glo->day_profit);
  }
  else{
    daytoken_prize = uint64_t (DIV_PROFIT*glo->day_profit/1000000000);
  }
  
  // profit dividend
  uint64_t dividend = daytokenpls.begin() == daytokenpls.end() ? 0 : daytoken_prize;
  for (auto itr = daytokenpls.begin(); itr != daytokenpls.end(); itr++) {

    uint64_t tokenbs = uint64_t(dividend * ((double)itr->token / (double)glo->day_token));
    auto useritr = players.find(itr->account);

    if(useritr != players.end()){
      players.modify(useritr, _self, [&](auto &p) {
       eosio_assert(p.tokenbonus + tokenbs >= p.tokenbonus,"integer overflow adding player tokenbonus");
       p.tokenbonus += tokenbs;
      });
    }
  }

   // find the max 5th,dayinviters->quantity,save to the table ivtranks
   auto quantity_index = dayinviters.get_index<N(quantity)>();
   auto fmx = quantity_index.end();
   for(int i=0; i<5; i++){
      if(fmx == quantity_index.begin()){break;}
      fmx--;
      ivtranks.emplace(_self, [&](auto& item){
        item.id = glo->ivtrank_id ;
        item.account = fmx->account;
	      item.quantity = fmx->quantity;	
		    item.time = now();
 	    });                  
      globalvars.modify(glo, _self, [&](auto &gl) { gl.ivtrank_id += 1; });
   }

  for (auto clean = daytokenpls.begin(); clean != daytokenpls.end();){         // erase table,dayptokenpls
    clean = daytokenpls.erase(clean);
  }   

  for (auto clr = dayinviters.begin(); clr != dayinviters.end();){         // erase table,dayinvites
    clr = dayinviters.erase(clr);
  }   
  
  globalvars.modify(glo, _self, [&](auto &gl) { gl.day_token = 0; }); 
  globalvars.modify(glo, _self, [&](auto &gl) { gl.day_amount = 0; });  
  globalvars.modify(glo, _self, [&](auto &gl) { gl.day_profit = 0; });
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
EOSIO_ABI(race_horse, (transfer)(initglo)(initair)(erasetab)(erasebets)(openhour)(openday)(opentoken))
