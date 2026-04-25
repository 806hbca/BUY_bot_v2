#include "flow_Analyst.h"
#include <string>
#include <thread>
#include <fstream>
#include "Analyst.h"

namespace nflow_Analyst {

 

	void flow_Analyst::add_figi(std::string& str)//добавление figi
	{
		figis.push_back(str);
	}

	void flow_Analyst::add_figi(std::vector<std::string>& mas)
	{
		figis = mas;
	}

	void flow_Analyst::set_token(std::string& str)// токена
	{
		nAnalyst::Analyst::set_token(str);
	}

	void flow_Analyst::end()//завершения программы
	{
		nAnalyst::Analyst::set_end(0);
	}

	bool flow_Analyst::correct_token(std::string& token)//проверки токена
	{
		return nAnalyst::Analyst::correct_token(token);
	}

	void flow_Analyst::start()//начала работы программы 
	{
		//std::thread end(nAnalyst::Analyst::the_end);
		for (int i = 0; i < figis.size(); i++)
		{
			mas_share.emplace_back(figis[i]);
		}
		for (auto& analyst : mas_share) {
			flow.emplace_back(&nAnalyst::Analyst::Analis_long_MACD_DAY, &analyst);
		}
		
		//end.join();
	}
	flow_Analyst::~flow_Analyst()
	{
		for (auto& thread : flow) {
			thread.join();
		}
	}
}