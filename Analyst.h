#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <deque>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <cmath>




namespace nAnalyst {

    using json = nlohmann::json;
    using namespace std;
    struct candle
    {
        candle(double close, string Time)
        {
            this->close = close;
            this->Time = Time;
        }
        candle(double close, string Time, double open)
        {
            this->close = close;
            this->Time = Time;
            this->open = open;
        }
        candle() {};
        double close;
        string Time;
        double open;
        friend ostream& operator<<(ostream&, candle);
    };


    struct one_MACD
    {
        one_MACD(string Tme, double signal, double macd)
        {
            this->Tme = Tme;
            this->signal = signal;
            this->macd = macd;
        }
        one_MACD() {};
        string Tme;
        double signal, macd;
        friend ostream& operator<<(ostream&, one_MACD);
    };

    class Analyst;

    void Analis_long_MACD_DAYf(Analyst&);
    void print(const deque<candle>&);
    void print(const map<string, int>&);
    void print(const deque<one_MACD>&);
    string getCurrentUTCTimeMinutes(int minut = 0);


    class Analyst
    {
        static string TOKEN;
        static string ID;
        static map<string, deque<candle>> current_list_figi;
        static bool not_end;

        string the_name_of_the_share;
        string instrumentId;//figi
        string instrumentUID;
        deque<candle> last_candle_interval_2_ho;
        json all_data_about_share;
        bool share_in_the_briefcase = 0;
        int count_share_in_the_briefcase = 0;
        double Purchase_price = -1;
        int limit_buy = 1;
        double profit = 0;
        ofstream FOUT;

        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
        static void set_ID();


        void To_find_out_information_using_figi();
        void set_instrumentUID();
        void set_the_name_of_the_share();


    public:
        static string get_ID();
        static void set_token(string&);
        static string get_token();
        static void get_list_figis();
        static void print_list_figis();
        static deque<string> get_list_figis_name();
        static void set_list_figis(list<string>&);
        static void set_list_figis(vector<string>&);
        static void set_list_figis(deque<string>&);
        static void set_list_figis(string);
        static void the_end();
        static void set_end(bool);
        static bool correct_token(string&);

        static void getLastPrices(
            const deque<string>& instrumentIds,
            bool del_first = 0,
            string lastPriceType = "LAST_PRICE_EXCHANGE",
            string instrumentStatus = "INSTRUMENT_STATUS_BASE");

        static deque<candle> getCandle_for_one_share_st(
            const string& instrumentId,
            const string& from,
            const string& to,
            const string& interval = "CANDLE_INTERVAL_1_MIN",
            const string& candleSourceType = "CANDLE_SOURCE_INCLUDE_WEEKEND",
            int limit = 2400);

        static void getCandle_for_all_shares_st(
            const string& from,
            const string& to,
            const string& interval = "CANDLE_INTERVAL_1_MIN",
            const string& candleSourceType = "CANDLE_SOURCE_INCLUDE_WEEKEND");



        void set_instrumentId(string&);
        string get_the_name_of_the_share();
        string get_instrumentId();
        string get_instrumentUID();
        deque<one_MACD> get_last_pair_MACD(const string& interval = "INDICATOR_INTERVAL_ONE_DAY");
        void buy();
        void sell();
        void buy_test(double);
        void sell_test(double);
        void Analis_long_MACD_DAY();
        void Analis_long_MACD_DAY_test(one_MACD, one_MACD, double);
        void Analis_long_MACD_DAY_test(one_MACD, one_MACD, deque<candle>&, int&);
        int lots_for_selling(double);
        int lots_for_buying(double);
        Analyst(const Analyst& other)
        {
            this->the_name_of_the_share = other.the_name_of_the_share;
            this->instrumentId = other.instrumentId;
            this->instrumentUID = other.instrumentUID;
            this->last_candle_interval_2_ho = other.last_candle_interval_2_ho;
            this->all_data_about_share = other.all_data_about_share;
            this->share_in_the_briefcase = other.share_in_the_briefcase;
            this->count_share_in_the_briefcase = other.count_share_in_the_briefcase;
            this->Purchase_price = other.Purchase_price;
            this->limit_buy = other.limit_buy;
            this->profit = other.profit;
            this->FOUT.open(other.instrumentId + ".txt", ios_base::app);
        }
        Analyst() {};
        Analyst(string& str)
        {
            this->set_instrumentId(str);
        }
        ~Analyst()
        {
            FOUT.close();
        }


        deque<candle> getCandle_for_one_share(
            const string& from,
            const string& to,
            const string& interval = "CANDLE_INTERVAL_1_MIN",
            const string& candleSourceType = "CANDLE_SOURCE_INCLUDE_WEEKEND",
            int limit = 2400);

        double getLastPrices(
            string lastPriceType = "LAST_PRICE_EXCHANGE",
            string instrumentStatus = "INSTRUMENT_STATUS_BASE");
        friend void Analis_long_MACD_DAYf(Analyst& share);
    };

}




