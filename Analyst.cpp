#include "Analyst.h"
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
#include <cmath>
#include <thread>
#include <Windows.h>
//#include "support.h"


namespace nAnalyst {

    using json = nlohmann::json;
    using namespace std;

    string Analyst::TOKEN = "";
    string Analyst::ID = "";
    bool Analyst::not_end = 1;
    map<string, deque<candle>> Analyst::current_list_figi;

    ostream& operator<<(ostream& out, one_MACD p)
    {
        cout << p.Tme << "\t" << p.signal << "\t" << p.macd << '\n';
        return out;
    }

    ostream& operator<<(ostream& out, candle p)
    {
        cout << p.close << "\t" << p.Time + '\n';
        return out;
    }
    //операторы для вывода в консоль информации о структурах

    void print(const deque<candle>& mas)
    {
        for (auto a : mas)
        {
            cout << a;
        }
    }

    void print(const map<string, int>& mas)
    {
        for (auto a : mas)
        {
            cout << a.first << " " << a.second << "\n";
        }
    }

    void print(const deque<one_MACD>& mas)
    {
        for (auto a : mas)
        {
            cout << a;
        }
    }

    string getCurrentUTCTimeMinutes(int minut) {
        using namespace std;

        auto now = chrono::system_clock::now();// Получаем текущее системное время

        auto now_time_t = chrono::system_clock::to_time_t(now);// Конвертируем в time_t (секунды с эпохи Unix)

        tm utc_tm;

        gmtime_s(&utc_tm, &now_time_t);  // Windows версия
        utc_tm.tm_min = utc_tm.tm_min + minut;  // прибавить/вычесть минуту(ы)
        mktime(&utc_tm);
        ostringstream oss;
        oss << put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S");
        return oss.str() + ".201Z";
    }

    string create_UUID()
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid UUID = generator();
        string uuid_str = boost::uuids::to_string(UUID);
        return uuid_str;
    }


    bool Analyst::correct_token(string& token)
    {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        // Инициализация cURL
        curl = curl_easy_init();
        if (curl) {
            // URL endpoint
            std::string url = "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.UsersService/GetAccounts";

            // JSON payload
            nlohmann::json payload = {
                {"status", "ACCOUNT_STATUS_OPEN"}
            };
            std::string payloadStr = payload.dump();

            // Заголовки
            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str()); // Добавьте ваш токен после Bearer

            // Настройка cURL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            // Выполнение запроса
            res = curl_easy_perform(curl);

            // Проверка на ошибки
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            else {
                // Вывод ответа
                bool p =  readBuffer.find("code")== std::string::npos&& 
                    readBuffer.find("error") == std::string::npos;
                return p;
            }

            // Очистка
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        return 0;
    }

    void Analyst::the_end()
    {
        while (not_end)
        {
            Sleep(1000 * 5);
        }
    }
    
    void Analyst::set_end(bool e)
    {
        not_end = e;
        cout << "END";
    }
    //методы для завершения программы

    void Analyst::set_the_name_of_the_share()
    {
        this->the_name_of_the_share = all_data_about_share["ticker"];
    }//Имя акции 

    void Analyst::set_instrumentId(string& Id)
    {
        this->instrumentId = Id;
        this->To_find_out_information_using_figi();
        this->set_instrumentUID();
        this->set_the_name_of_the_share();
        this->FOUT.open(this->instrumentId + ".txt", std::ios_base::app);
    }// Получение Id и задание инициализация прочих полей 

    void Analyst::set_token(string& token)
    {
        TOKEN = token;
        set_ID();
    }

    string Analyst::get_ID()
    {
        return ID;
    }
    //методы для заврешения программы
    void Analyst::set_ID()
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        json payload = {
            {"status", "ACCOUNT_STATUS_OPEN"}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.UsersService/GetAccounts");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }
            string ans;

            try {
                json response = json::parse(readBuffer);
                ans = response["accounts"][0]["id"];
            }
            catch (const json::parse_error& e) {
                cerr << "JSON parse error: " << e.what() << endl;
                cout << "Raw response:" << endl;
                cout << readBuffer << endl;
            }
            ID = ans;

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
    }//установка ID

    size_t Analyst::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }//метод для обработки содержимого json запросов

    string Analyst::get_the_name_of_the_share()
    {
        return this->the_name_of_the_share;
    }// получение имени акции

    string Analyst::get_instrumentId()
    {
        return this->instrumentId;
    }//получение Fige акции

    string Analyst::get_token()
    {
        return TOKEN;
    }//получение токена


    void Analyst::getLastPrices(//получение цен акции, статический
        const deque<string>& instrumentIds,
        bool del_first,
        string lastPriceType,
        string instrumentStatus) {

        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        json payload = {
            {"instrumentId", instrumentIds},
            {"lastPriceType", lastPriceType},
            {"instrumentStatus", instrumentStatus}
        };// Формируем JSON запрос

        curl = curl_easy_init();
        if (curl) {
            // Устанавливаем URL
            curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox-invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.MarketDataService/GetLastPrices");

            struct curl_slist* headers = nullptr;// Устанавливаем заголовки
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string payloadStr = payload.dump();// Устанавливаем POST данные
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);// Устанавливаем callback
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);// Выполняем запрос

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }// Проверяем ошибки

            curl_slist_free_all(headers);// Очищаем
            curl_easy_cleanup(curl);
        }

        // Парсим JSON ответ
        try {
            json list_prices = json::parse(readBuffer);
            for (auto a : list_prices["lastPrices"])
            {
                string figi = a["figi"];
                double close = stod((string)a["price"]["units"]) + pow(10, -9) * a["price"]["nano"];
                if (del_first && current_list_figi[figi].size() != 0)
                {
                    current_list_figi[figi].pop_front();
                }
                current_list_figi[figi].emplace_back(close, (string)a["time"]);
            }

        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    double Analyst::getLastPrices(
        string lastPriceType,
        string instrumentStatus)
    {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        deque<string> mas = { this->instrumentId };

        json payload = {
            {"instrumentId", mas},
            {"lastPriceType", lastPriceType},
            {"instrumentStatus", instrumentStatus}
        };// Формируем JSON запрос

        curl = curl_easy_init();
        if (curl) {
            // Устанавливаем URL
            curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox-invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.MarketDataService/GetLastPrices");

            struct curl_slist* headers = nullptr;// Устанавливаем заголовки
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string payloadStr = payload.dump();// Устанавливаем POST данные
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);// Устанавливаем callback
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);// Выполняем запрос

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }// Проверяем ошибки

            curl_slist_free_all(headers);// Очищаем
            curl_easy_cleanup(curl);
        }

        // Парсим JSON ответ
        try {
            json list_price = json::parse(readBuffer)["lastPrices"][0];
            double close = stod((string)list_price["price"]["units"]) + pow(10, -9) * list_price["price"]["nano"];
            return  close;
        }
        catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return -1;
        }
    }//получение цен акции

   

    void Analyst::To_find_out_information_using_figi()//метод для получения информации об инструменте 
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        json payload = {
            {"idType", "INSTRUMENT_ID_TYPE_FIGI"},
            {"classCode", "string"},
            {"id", instrumentId}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://sandbox-invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.InstrumentsService/GetInstrumentBy");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        try {
            all_data_about_share = json::parse(readBuffer)["instrument"];
        }
        catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
            //all_data_about_share = nullptr;
        }
    }

    void Analyst::set_instrumentUID()//инициализация UID инструмента
    {
        instrumentUID = all_data_about_share["uid"];
    }

    string Analyst::get_instrumentUID()//получение UID инструмента
    {
        return instrumentUID;
    }

    deque<one_MACD> Analyst::get_last_pair_MACD(const string& interval)//получение списка значений MACD
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        // Prepare the JSON payload for MACD analysis
        json payload = {
            {"indicatorType", "INDICATOR_TYPE_MACD"},
            {"instrumentUid", this->instrumentUID},
            {"from", getCurrentUTCTimeMinutes(-5 * 60 * 24)},
            {"to", getCurrentUTCTimeMinutes()},
            {"interval", interval},
            {"typeOfPrice", "TYPE_OF_PRICE_CLOSE"},
            {"length", 96},
            {"deviation", {
                {"deviationMultiplier", {
                    {"nano", 0},
                    {"units", "0"}
                }}
            }},
            {"smoothing", {
                {"fastLength", 12},
                {"slowLength", 26},
                {"signalSmoothing", 9}
            }}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.MarketDataService/GetTechAnalysis");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        deque<one_MACD> ans;
        try {
            json response = json::parse(readBuffer);
            for (auto a : response["technicalIndicators"])
            {
                ans.emplace_back(
                    a["timestamp"],
                    stod((string)a["signal"]["units"]) + a["signal"]["nano"] * pow(10, -9),
                    stod((string)a["macd"]["units"]) + a["macd"]["nano"] * pow(10, -9)
                );
            }

        }
        catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
            cout << "Raw response:" << endl;
            cout << readBuffer << endl;
        }
        return ans;
    }

    void Analyst::Analis_long_MACD_DAY_test(one_MACD pre, one_MACD now, double current_price)//функция для анализа на тестовых данных
    {
        //deque<one_MACD> mas = this->get_last_pair_MACD();

        //one_MACD pre = mas[mas.size() - 2], now = mas[mas.size() - 1];
        if (!this->share_in_the_briefcase)
        {
            if (pre.macd < pre.signal && now.macd >= now.signal)
            {
                this->buy_test(current_price);
            }
        }
        else
        {
            if (pre.macd > pre.signal && now.macd <= now.signal && current_price > (Purchase_price * 1.004 + (current_price - Purchase_price) * 0.13))
            {
                this->sell_test(current_price);
            }
        }
    }
    //функция для анализа на тестовых данных
    void Analyst::Analis_long_MACD_DAY_test(one_MACD pre, one_MACD now, deque<candle>& last_candle_interval_2_ho, int& index_start)
    {
        string tm = now.Tme.substr(0, 10);
        while (last_candle_interval_2_ho[index_start].Time.substr(0, 10) != tm)
        {
            index_start++;
        }
        if (index_start == last_candle_interval_2_ho.size())
        {
            return;
        }

        if (!this->share_in_the_briefcase)
        {
            if (pre.macd < pre.signal && now.macd >= now.signal)
            {
                cout << last_candle_interval_2_ho[index_start].Time << " :";
                this->buy_test(last_candle_interval_2_ho[index_start].close);
            }
        }
        else
        {
            if (pre.macd > pre.signal && now.macd <= now.signal)
            {
                cout << last_candle_interval_2_ho[index_start].Time << " :";
                this->sell_test(last_candle_interval_2_ho[index_start].close);
                /*if (last_candle_interval_2_ho[index_start].close > Purchase_price * 1.006)
                {
                    cout << last_candle_interval_2_ho[index_start].Time << " :";
                    this->sell_test(last_candle_interval_2_ho[index_start].close);
                }
                else if (time_after_buy > 8)
                {
                    cout << last_candle_interval_2_ho[index_start].Time << " :";
                    this->sell_test(last_candle_interval_2_ho[index_start].close);
                }*/

            }
        }

        if (index_start == 1 || index_start == 2)
        {
            return;
        }

        /*if (last_candle_interval_2_ho[index_start].close > Purchase_price * 1.1&& this->share_in_the_briefcase)
        {
            cout << last_candle_interval_2_ho[index_start].Time << " :";
            this->sell_test(last_candle_interval_2_ho[index_start].close);
        }*/


        //while (last_candle_interval_2_ho[index_start].Time.substr(0, 10) ==
        //    tm)
        //{
        //    index_start++;
        //    if (last_candle_interval_2_ho[index_start].close > (Purchase_price * 1.008 +
        //        (last_candle_interval_2_ho[index_start].close - Purchase_price) * 0.13) && share_in_the_briefcase)
        //    {
        //        cout << last_candle_interval_2_ho[index_start].Time<<" :";
        //        this->sell_test(last_candle_interval_2_ho[index_start].close);
        //    }
        //    /*double v1, v2, v3;
        //    v1 = (last_candle_interval_2_ho[index_start].close - last_candle_interval_2_ho[index_start-1].close) / 2;
        //    v2 = (last_candle_interval_2_ho[index_start - 1].close - last_candle_interval_2_ho[index_start - 2].close) / 2;
        //    v3 = (last_candle_interval_2_ho[index_start - 2].close - last_candle_interval_2_ho[index_start - 3].close) / 2;
        //    if (v1 < 0 && v2 < 0 && v3 < 0 && share_in_the_briefcase)
        //    {
        //        this->sell_test(last_candle_interval_2_ho[index_start].close);
        //    }*/
        //}
    }
    //функция для анализа и покупки/продажи акций
    void Analyst::Analis_long_MACD_DAY()
    {

        string tm = getCurrentUTCTimeMinutes();

        int hour = stoi(tm.substr(11, 2));
        int minuts = stoi(tm.substr(14, 2));
        cout << "\nIn method " << this->the_name_of_the_share << '\n';
        /*
        if (hour > 4)
        {
            cout << 24 - hour - 1 + 4 << "  " << 60 - minuts<<'\n';
            Sleep(((24 - hour - 1 + 4) * 60 + 60 - minuts) * 60 * 1000);
            Sleep(60 * 1000);
        }
        else if (hour < 4)
        {
            Sleep(((4- hour -1) * 60 + 60 - minuts) * 60 * 1000);
            Sleep(60 * 1000);
        }*/
        while (not_end)
        {
            double current_price = this->getLastPrices();
            deque<one_MACD> mas = this->get_last_pair_MACD();
            one_MACD pre = mas[mas.size() - 2], now = mas[mas.size() - 1];

            if (!this->share_in_the_briefcase)
            {
                if (pre.macd < pre.signal && now.macd >= now.signal)
                {
                    this->buy();
                }
            }
            else
            {
                if (pre.macd > pre.signal && now.macd <= now.signal)
                {
                    this->sell();
                }
            }
            for (int i = 0; i < 60 * 60 * 24 && not_end; i++)
            {
                Sleep(1000);
            }
        }
        cout << "\nEND for " << this->the_name_of_the_share << "\n";
    }


    
    
    int Analyst::lots_for_selling(double price)//определение числа возможных лотов для продажи
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        double whole_part = floor(price);
        double fractional_part = price - whole_part;

        string whole_str = to_string(static_cast<int>(whole_part));

        ostringstream oss;
        oss << fixed << setprecision(9) << fractional_part;
        string frac_str_full = oss.str().substr(2); // Skip "0."

        frac_str_full.resize(9, '0');
        json payload = {
            {"accountId", ID},
            {"instrumentId", instrumentId},
            {"price", {
                {"nano", stoi(frac_str_full)},
                {"units", whole_str}
            }}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str()); // Add your token here

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.OrdersService/GetMaxLots");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
                return 1;
            }

            try {
                json response = json::parse(readBuffer);
                return stoi((string)response["sellLimits"]["sellMaxLots"]);
            }
            catch (const json::parse_error& e) {
                cerr << "JSON parse error: " << e.what() << endl;
                cout << "Raw response:" << endl;
                cout << readBuffer << endl;
                return 1;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        return 0;
    }

    int Analyst::lots_for_buying(double price)//определение числа возможных лотов для покупки 
    {
        CURL* curl;
        CURLcode res;
        string readBuffer;

        double whole_part = floor(price);
        double fractional_part = price - whole_part;

        string whole_str = to_string(static_cast<int>(whole_part));

        ostringstream oss;
        oss << fixed << setprecision(9) << fractional_part;
        string frac_str_full = oss.str().substr(2); // Skip "0."

        frac_str_full.resize(9, '0');

        json payload = {
            {"accountId", ID},
            {"instrumentId", instrumentId},
            {"price", {
                {"nano", stoi(frac_str_full)},
                {"units", whole_str}
            }}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str()); // Add your token here

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.OrdersService/GetMaxLots");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
                return 1;
            }

            try {
                json response = json::parse(readBuffer);
                int t = stoi((string)response["buyLimits"]["buyMaxLots"]);
                if (t > limit_buy)
                {
                    return limit_buy;
                }
                else
                {
                    return t;
                }
            }
            catch (const json::parse_error& e) {
                cerr << "JSON parse error: " << e.what() << endl;
                cout << "Raw response:" << endl;
                cout << readBuffer << endl;
                return 1;
            }

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }

        curl_global_cleanup();
        return 0;
    }

    void Analyst::buy()
    {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;
        double price = this->getLastPrices();

        double whole_part = floor(price);
        double fractional_part = price - whole_part;

        string whole_str = to_string(static_cast<int>(whole_part));

        ostringstream oss;
        oss << fixed << setprecision(9) << fractional_part;
        string frac_str_full = oss.str().substr(2);

        frac_str_full.resize(9, '0');

        string UUID = create_UUID();

        json payload = {
            {"instrumentId", this->instrumentId},
            {"quantity", to_string(this->lots_for_buying(price))},
            {"price", {
                {"nano", stoi(frac_str_full)},
                {"units", whole_str}
            }},
            {"direction", "ORDER_DIRECTION_BUY"},
            {"accountId", ID},
            {"orderType", "ORDER_TYPE_LIMIT"},
            {"orderId", UUID},
            {"timeInForce", "TIME_IN_FORCE_DAY"},
            {"priceType", "PRICE_TYPE_CURRENCY"}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.OrdersService/PostOrderAsync");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                return;
            }

            try {
                json response = json::parse(readBuffer);
                FOUT << "BUY " + getCurrentUTCTimeMinutes() << " " <<
                    UUID << " " << (string)response["tradeIntentId"] << " "
                    << (string)response["executionReportStatus"] << " "
                    << price << "\n";
                cout << "BUY " + getCurrentUTCTimeMinutes() << " " <<
                    UUID << " " << (string)response["tradeIntentId"] << " "
                    << (string)response["executionReportStatus"] << " "
                    << price << "\n";
            }
            catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                std::cout << "Raw response:" << std::endl;
                std::cout << readBuffer << std::endl;
                return;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    void Analyst::sell()
    {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;
        double price = this->getLastPrices();

        double whole_part = floor(price);
        double fractional_part = price - whole_part;

        string whole_str = to_string(static_cast<int>(whole_part));

        ostringstream oss;
        oss << fixed << setprecision(9) << fractional_part;
        string frac_str_full = oss.str().substr(2);

        frac_str_full.resize(9, '0');

        string UUID = create_UUID();

        int count_lots = this->lots_for_selling(price);

        json payload = {
            {"instrumentId", this->instrumentId},
            {"quantity", to_string(count_lots)},
            {"price", {
                {"nano", stoi(frac_str_full)},
                {"units", whole_str}
            }},
            {"direction", "ORDER_DIRECTION_SELL"},
            {"accountId", ID},
            {"orderType", "ORDER_TYPE_LIMIT"},
            {"orderId", UUID},
            {"timeInForce", "TIME_IN_FORCE_DAY"},
            {"priceType", "PRICE_TYPE_CURRENCY"}
        };

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.OrdersService/PostOrderAsync");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            std::string payloadStr = payload.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                return;
            }

            try {
                json response = json::parse(readBuffer);
                FOUT << "SELL " + getCurrentUTCTimeMinutes() << " " <<
                    UUID << " " << (string)response["tradeIntentId"] << " "
                    << (string)response["executionReportStatus"] << " "
                    << price << " Lots:"<<count_lots<<"\n";
                cout << "SELL " + getCurrentUTCTimeMinutes() << " " <<
                    UUID << " " << (string)response["tradeIntentId"] << " "
                    << (string)response["executionReportStatus"] << " "
                    << price << " Lots:" << count_lots << "\n";
            }
            catch (const json::parse_error& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
                std::cout << "Raw response:" << std::endl;
                std::cout << readBuffer << std::endl;
                return;
            }
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }
















    void Analyst::buy_test(double current_price)
    {
        cout << "\nbuy with price: " << current_price << "\n\n";
        share_in_the_briefcase = 1;
        Purchase_price = current_price;
        count_share_in_the_briefcase = limit_buy;
    }

    void Analyst::sell_test(double current_price)
    {
        cout << "\nsell with price: " << current_price << "\n";
        profit += (current_price - (Purchase_price * 1.006));
        cout << profit << "\n\n";
        share_in_the_briefcase = 0;
        Purchase_price = -1;
        count_share_in_the_briefcase = 0;
    }


    void Analis_long_MACD_DAYf(Analyst& share)
    {
        string tm = getCurrentUTCTimeMinutes();

        int hour = stoi(tm.substr(11, 2));
        int minuts = stoi(tm.substr(14, 2));
        if (hour > 4)
        {
            Sleep(((24 - hour - 1 + 4) * 60 + 60 - minuts) * 60 * 1000);
            Sleep(60 * 1000);
        }
        while (Analyst::not_end)
        {
            double current_price = share.getLastPrices();
            deque<one_MACD> mas = share.get_last_pair_MACD();
            one_MACD pre = mas[mas.size() - 2], now = mas[mas.size() - 1];

            if (!share.share_in_the_briefcase)
            {
                if (pre.macd < pre.signal && now.macd >= now.signal)
                {
                    share.buy();
                }
            }
            else
            {
                if (pre.macd > pre.signal && now.macd <= now.signal)
                {
                    share.sell();
                }
            }
            Sleep(1000 * 60 * 60 * 24);
        }
    }

    void Analyst::get_list_figis()
    {
        for (auto a : current_list_figi)
        {
            cout << a.first << "\n";
        }
    }

    void Analyst::set_list_figis(list<string>& mas)
    {
        for (auto a : mas)
        {
            deque<candle> per;
            current_list_figi.insert({ a, per });
        }
    }
    void Analyst::set_list_figis(deque<string>& mas)
    {
        for (auto a : mas)
        {
            deque<candle> per;
            current_list_figi.insert({ a, per });
        }
    }
    void Analyst::set_list_figis(vector<string>& mas)
    {
        for (auto a : mas)
        {
            deque<candle> per;
            current_list_figi.insert({ a, per });
        }
    }
    void Analyst::set_list_figis(string file)
    {
        ifstream fin(file);
        while (!fin.eof())
        {
            string a;
            fin >> a;
            deque<candle> per;
            current_list_figi.insert({ a, per });
        }
        fin.close();
    }
    //инициализация current_list_figi и её перегрузки
    deque<string> Analyst::get_list_figis_name()
    {
        deque<string> ans;
        for (auto a : current_list_figi)
        {
            ans.push_back(a.first);
        }
        return ans;
    }

    void Analyst::print_list_figis()
    {
        cout << "\n";
        for (auto a : current_list_figi)
        {
            cout << a.first << ":\n\n";
            for (auto b : a.second)
            {
                cout << b.close << "  " << b.Time << "\n";
            }
            cout << "\n";
        }
    }

    deque<candle> Analyst::getCandle_for_one_share(
        const string& from,
        const string& to,
        const string& interval,
        const string& candleSourceType,
        int limit) {

        CURL* curl;
        CURLcode res;
        string readBuffer;

        // Подготовка JSON payload
        json payload = {
            {"from", from},
            {"to", to},
            {"interval", interval},
            {"instrumentId", instrumentId},
            {"candleSourceType", candleSourceType},
            {"limit", limit}
        };

        curl = curl_easy_init();
        if (curl) {
            // Устанавливаем URL
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.MarketDataService/GetCandles");

            struct curl_slist* headers = nullptr;// Устанавливаем заголовки
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            string payloadStr = payload.dump();//преобразует в строку
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());//отпраляем запрос
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);// Устанавливаем callback функцию
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);// Выполняем запрос

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }// Проверяем на ошибки

            curl_slist_free_all(headers);// Очищаем
            curl_easy_cleanup(curl);
        }
        deque<candle> ans;
        try {
            json listcandles = json::parse(readBuffer);
            for (auto a : listcandles["candles"])
            {
                double close = stod((string)a["close"]["units"]) + pow(10, -9) * a["close"]["nano"];
                string Time = a["time"];
                double open = stod((string)a["open"]["units"]) + pow(10, -9) * a["open"]["nano"];
                ans.emplace_back(close, Time, open);
            }
            return ans;
        }
        catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << std::endl;
            return ans;
        }
    }

    deque<candle> Analyst::getCandle_for_one_share_st(
        const string& instrumentId,
        const string& from,
        const string& to,
        const string& interval,
        const string& candleSourceType,
        int limit) {

        CURL* curl;
        CURLcode res;
        string readBuffer;

        // Подготовка JSON payload
        json payload = {
            {"from", from},
            {"to", to},
            {"interval", interval},
            {"instrumentId", instrumentId},
            {"candleSourceType", candleSourceType},
            {"limit", limit}
        };

        curl = curl_easy_init();
        if (curl) {
            // Устанавливаем URL
            curl_easy_setopt(curl, CURLOPT_URL, "https://invest-public-api.tinkoff.ru/rest/tinkoff.public.invest.api.contract.v1.MarketDataService/GetCandles");

            struct curl_slist* headers = nullptr;// Устанавливаем заголовки
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + TOKEN).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            string payloadStr = payload.dump();//преобразует в строку
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());//отпраляем запрос
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payloadStr.length());

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);// Устанавливаем callback функцию
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            res = curl_easy_perform(curl);// Выполняем запрос

            if (res != CURLE_OK) {
                cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            }// Проверяем на ошибки

            curl_slist_free_all(headers);// Очищаем
            curl_easy_cleanup(curl);
        }
        deque<candle> ans;
        try {
            json listcandles = json::parse(readBuffer);
            for (auto a : listcandles["candles"])
            {
                double close = stod((string)a["close"]["units"]) + pow(10, -9) * a["close"]["nano"];
                string Time = a["time"];
                double open = stod((string)a["open"]["units"]) + pow(10, -9) * a["open"]["nano"];
                ans.emplace_back(close, Time, open);
            }
            return ans;
        }
        catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << std::endl;
            return ans;
        }
    }

    void Analyst::getCandle_for_all_shares_st(
        const string& from,
        const string& to,
        const string& interval,
        const string& candleSourceType)
    {
        for (auto a : Analyst::get_list_figis_name())
        {
            current_list_figi[a] = Analyst::getCandle_for_one_share_st(a, from, to, interval, candleSourceType);
        }
    }
}