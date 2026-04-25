#pragma once
#include <string>
#include <vector>
#include <thread>
#include "Analyst.h"
//using namespace std;

namespace nflow_Analyst {
	class flow_Analyst
	{
		std::vector<std::string> figis;
		std::vector<nAnalyst::Analyst> mas_share;
		std::vector<std::thread> flow;
	

	public:
		void add_figi(std::string&);
		void add_figi(std::vector<std::string>&);
		void set_token(std::string&);
		void start();
		void end();
		bool correct_token(std::string&);
		~flow_Analyst();
	};
}

