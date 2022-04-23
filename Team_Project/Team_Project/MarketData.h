//
//  MarketData.h
//  FRE7831_Lab1
//
//  Created by Hanji Gu on 4/10/22.
//

#ifndef MarketData_h
#define MarketData_h

#pragma once
#include <string>


class Stock;    // forward declaration


int PullMarketData(const std::string& url_request, std::string& read_buffer);


int PopulateDailyTrades(const std::string& read_buffer,
                    Stock& stock);
#endif /* MarketData_h */
