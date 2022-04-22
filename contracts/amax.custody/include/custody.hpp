#include "custodydb.hpp"

using namespace std;
using namespace wasm::db;

static constexpr bool DEBUG = true;

#define WASM_FUNCTION_PRINT_LENGTH 50

#define AMAX_LOG( debug, exception, ... ) {  \
if ( debug ) {                               \
   std::string str = std::string(__FILE__); \
   str += std::string(":");                 \
   str += std::to_string(__LINE__);         \
   str += std::string(":[");                \
   str += std::string(__FUNCTION__);        \
   str += std::string("]");                 \
   while(str.size() <= WASM_FUNCTION_PRINT_LENGTH) str += std::string(" ");\
   eosio::print(str);                                                             \
   eosio::print( __VA_ARGS__ ); }}

#define div(a, b) divide_decimal(a, b, PRECISION_1)
#define mul(a, b) multiply_decimal(a, b, PRECISION_1)

#define high_div(a, b) divide_decimal(a, b, HIGH_PRECISION_1)
#define high_mul(a,b ) multiply_decimal(a, b, HIGH_PRECISION_1)

class [[eosio::contract("amax.custody")]] custody: public eosio::contract {
private:
    dbc                 _db;
    global_singleton    _global;
    global_t            _gstate;

public:
    using contract::contract;

    custody(eosio::name receiver, eosio::name code, datastream<const char*> ds):
        contract(receiver, code, ds),
        _global(get_self(), get_self().value),
        _db(get_self())
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    [[eosio::action]] void setconfig(const asset &plan_fee, const name &fee_receiver);
    [[eosio::action]] void addplan(const name& owner, const string& title, const name& asset_contract, const symbol& asset_symbol, const uint64_t& unlock_interval_days, const int64_t& unlock_times);
    [[eosio::action]] void setplanowner(const name& owner, const uint64_t& plan_id, const name& new_owner);
    [[eosio::action]] void enableplan(const name& owner, const uint64_t& plan_id, bool enabled);
    [[eosio::action]] void delplan(const name& owner, const uint64_t& plan_id); //by maintainer only

    [[eosio::action]] void addissue(const name& issuer, const name& receiver, uint64_t plan_id, uint64_t first_unlock_days);
    /**
     * @param memo - memo format:
     *               plan:${plan_id}, pay plan fee, Eg: "plan:" or "plan:1"
     *               issue:${issue_id}, deposit token to issue, Eg: "issue:" or "issue:1"
     */
    [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);
    [[eosio::action]] void unlock(const name& unlocker, const uint64_t& plan_id, const uint64_t& issue_id);
    [[eosio::action]] void endissue(const name& issuer, const uint64_t& plan_id, const uint64_t& issue_id); //run by plan owner only
private:
    void internal_unlock(const name& actor, const uint64_t& plan_id,
                         const uint64_t& issue_id, bool is_end_action);
}; //contract custody