//
//  main.cpp
//  Team_Project
//
//  Created by Hanji Gu on 4/19/22.
//

#include <iostream>
#include "PairTrade.h"
#include "Database.h"
#include "Util.hpp"
#include "MarketData.h"

int main(int argc, const char * argv[]) {

    sqlite3* db = NULL;
    string database = "PairTrading.db";
    string trading_pair = "PairTrading.txt";
    vector<StockPairPrices> pairs_vec;
    vector<string> pair_one_vec;
    vector<string> pair_two_vec;
    map<string, Stock> stockMapOne;
    map<string, Stock> stockMapTwo;

    
    if (OpenDatabase(database.c_str(), db) != 0)
        return -1;
    
    bool bCompleted = false;
    char selection;

    while (!bCompleted)
    {
        std::cout << endl;
        std::cout << "Menu" << endl;
        std::cout << "========" << endl;
        std::cout << "A - Create and Populate Pair Table" << endl;
        std::cout << "B - Retrieve and Populate Historical Data for Each Stock" << endl;
        std::cout << "C - Create PairPrices Table" << endl;
        std::cout << "D - Calculate Volatility" << endl;
        std::cout << "E - Back Test" << endl;
        std::cout << "F - Calculate Profit and Loss for Each Pair" << endl;
        std::cout << "G - Manual Testing" << endl;
        std::cout << "H - Drop All the Tables" << endl;
        std::cout << "X - Exit" << endl << endl;
        
        cout << "Enter Selection(Case Insensitive): ";
        cin >> selection;
        cout << endl;
        
        selection = toupper(selection);
        switch (toupper(selection))
        {
            case 'A': {
                string sql_DropaTable = "DROP TABLE IF EXISTS StockPairs";
                if (DropTable(db, sql_DropaTable.c_str()) == -1)
                    return -1;
                
                cout << "Creating StockPairs table" << endl;
                string sql_CreateTable = string("CREATE TABLE IF NOT EXISTS StockPairs ")
                                                + "(id INT NOT NULL,"
                                                + "Symbol1 CHAR(20) NOT NULL,"
                                                + "Symbol2 CHAR(20) NOT NULL,"
                                                + "Volatility FLOAT NOT NULL,"
                                                + "profit_loss FLOAT NOT NULL,"
                                                + "PRIMARY KEY(Symbol1, Symbol2)"
                                                + ");";
                if (ExecuteSQL(db, sql_CreateTable.c_str()) == -1)
                {
                    return -1;
                }
                else
                {
                    cout << "Successfully created table StockPairs!" << endl;
                }
                vector<pair<string, string>> trading_pairs = GetPairs(trading_pair);
                int count = 0;
                for (vector<pair<string, string>>::iterator itr = trading_pairs.begin(); itr != trading_pairs.end(); itr++)
                {
                    StockPairPrices perPair(*itr);
                    pairs_vec.push_back(perPair);
                    char sql_Insert_Line[200] = "";
                    sprintf(sql_Insert_Line, "INSERT INTO StockPairs(id, symbol1, symbol2, volatility, profit_loss) VALUES(%d, \"%s\", \"%s\", %f, %f)",
                            ++count,
                            perPair.GetStockPair().first.c_str(),
                            perPair.GetStockPair().second.c_str(),
                            perPair.GetVolatility(),
                            0.0);

                    if (ExecuteSQL(db, sql_Insert_Line) == -1)
                    {
                        cout << perPair.GetStockPair().first << " and "
                             << perPair.GetStockPair().second << " fail to insert to StockPair table!" << endl;
                        return -1;
                    }
                    else
                    {
                        cout << count << " Successfully inserting " << perPair.GetStockPair().first
                             << " and " << perPair.GetStockPair().second
                             << " to StockPairs!" << endl;
                    }
                    pair_one_vec.push_back(perPair.GetStockPair().first);
                    pair_two_vec.push_back(perPair.GetStockPair().second);
                }
                break;
            }

            case 'B': {
                string sql_DropaTable = "DROP TABLE IF EXISTS PairOnePrices";
                if (DropTable(db, sql_DropaTable.c_str()) == -1)
                    return -1;
                sql_DropaTable = "DROP TABLE IF EXISTS PairTwoPrices";
                if (DropTable(db, sql_DropaTable.c_str()) == -1)
                    return -1;

                cout << "Creating PairOnePrices table" << endl;
                string sql_CreateTable = string("CREATE TABLE IF NOT EXISTS PairOnePrices ")
                                                + "(symbol CHAR(20) NOT NULL,"
                                                + "date CHAR(20) NOT NULL,"
                                                + "open REAL NOT NULL,"
                                                + "high REAL NOT NULL,"
                                                + "low REAL NOT NULL,"
                                                + "close REAL NOT NULL,"
                                                + "adjusted_close REAL NOT NULL,"
                                                + "volume INT NOT NULL,"
                                                + "PRIMARY KEY(Symbol, Date)"
                                                + ");";
                if (ExecuteSQL(db, sql_CreateTable.c_str()) == -1)
                {
                    return -1;
                }
                else
                {
                    cout << "Successfully created table PairOnePrices!" << endl;
                }
                
                cout << "Creating PairTwoPrices table" << endl;
                sql_CreateTable = string("CREATE TABLE IF NOT EXISTS PairTwoPrices ")
                                                + "(symbol CHAR(20) NOT NULL,"
                                                + "date CHAR(20) NOT NULL,"
                                                + "open REAL NOT NULL,"
                                                + "high REAL NOT NULL,"
                                                + "low REAL NOT NULL,"
                                                + "close REAL NOT NULL,"
                                                + "adjusted_close REAL NOT NULL,"
                                                + "volume INT NOT NULL,"
                                                + "PRIMARY KEY(Symbol, Date)"
                                                + ");";
                if (ExecuteSQL(db, sql_CreateTable.c_str()) == -1)
                {
                    return -1;
                }
                else
                {
                    cout << "Successfully created table PairTwoPrices!" << endl;
                }
                
                string sConfigFile = "config.csv";
                map<string, string> config_map = ProcessConfigData(sConfigFile);
                
                string daily_url_common = config_map["daily_url_common"];
                string start_date = config_map["start_date"];
                string end_date = config_map["end_date"];
                string api_token = config_map["api_token"];
                
                for (vector<string>::iterator itr = pair_one_vec.begin(); itr != pair_one_vec.end(); itr++)
                {
                    string daily_readBuffer;
                    string intraday_readBuffer;

                    //use constructor Stock(string symbol_) :symbol(symbol_)
                    Stock perStock(*itr);
                    string daily_url_request = daily_url_common + *itr + ".US?"
                                                + "from=" + start_date + "&to=" + end_date
                                                + "&api_token=" + api_token + "&period=d&fmt=json";
                    
                    if (PullMarketData(daily_url_request, daily_readBuffer) != 0)
                    {
                        return -1;
                    }
                    if (PopulateDailyTrades(daily_readBuffer, perStock) != 0)
                    {
                        return -1;
                    }
                    stockMapOne[*itr] = perStock;
                }
                
                for (vector<string>::iterator itr = pair_two_vec.begin(); itr != pair_two_vec.end(); itr++)
                {
                    string daily_readBuffer;
                    string intraday_readBuffer;

                    //use constructor Stock(string symbol_) :symbol(symbol_)
                    Stock perStock(*itr);
                    string daily_url_request = daily_url_common + *itr + ".US?"
                                                + "from=" + start_date + "&to=" + end_date
                                                + "&api_token=" + api_token + "&period=d&fmt=json";
                    
                    if (PullMarketData(daily_url_request, daily_readBuffer) != 0)
                    {
                        return -1;
                    }
                    if (PopulateDailyTrades(daily_readBuffer, perStock) != 0)
                    {
                        return -1;
                    }
                    stockMapTwo[*itr] = perStock;
                }
                
                for (map<string, Stock>::iterator itr_stocks = stockMapOne.begin(); itr_stocks != stockMapOne.end(); itr_stocks++)
                {
                    string curr_symbol = itr_stocks->first;
                    // curr_stock object contains a vector of DailyTrades and a vector of IntradayTrades
                    Stock& curr_stock = itr_stocks->second;
                    vector<TradeData> dailyTrade_vec = curr_stock.getTrades();
                    for (vector<TradeData>::iterator itr_DailyTrade = dailyTrade_vec.begin();
                         itr_DailyTrade != dailyTrade_vec.end(); itr_DailyTrade++)
                    {
                        char sql_Insert_Line[200] = "";
                        sprintf(sql_Insert_Line, "INSERT INTO PairOnePrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %ld)",
                                curr_symbol.c_str(),
                                itr_DailyTrade->GetDate().c_str(),
                                itr_DailyTrade->GetOpen(),
                                itr_DailyTrade->GetHigh(),
                                itr_DailyTrade->GetLow(),
                                itr_DailyTrade->GetClose(),
                                itr_DailyTrade->GetAdjustedClose(),
                                itr_DailyTrade->GetVolume());
                        if (ExecuteSQL(db, sql_Insert_Line) == -1)
                        {
                            cout << curr_symbol << " "
                                 << itr_DailyTrade->GetDate()
                                 << " fail to insert to PairOnePrices table!" << endl;
                            return -1;
                        }
                        else
                        {
                            cout << curr_symbol << " " << itr_DailyTrade->GetDate() << " Successfully inserted to PairOnePrices table!" << endl;
                        }
                    }
                }

                for (map<string, Stock>::iterator itr_stocks = stockMapTwo.begin(); itr_stocks != stockMapTwo.end(); itr_stocks++)
                {
                    string curr_symbol = itr_stocks->first;
                    // curr_stock object contains a vector of DailyTrades and a vector of IntradayTrades
                    Stock& curr_stock = itr_stocks->second;
                    vector<TradeData> dailyTrade_vec = curr_stock.getTrades();
                    for (vector<TradeData>::iterator itr_DailyTrade = dailyTrade_vec.begin();
                         itr_DailyTrade != dailyTrade_vec.end(); itr_DailyTrade++)
                    {
                        char sql_Insert_Line[200] = "";
                        sprintf(sql_Insert_Line, "INSERT INTO PairTwoPrices(symbol, date, open, high, low, close, adjusted_close, volume) VALUES(\"%s\", \"%s\", %f, %f, %f, %f, %f, %ld)",
                                curr_symbol.c_str(),
                                itr_DailyTrade->GetDate().c_str(),
                                itr_DailyTrade->GetOpen(),
                                itr_DailyTrade->GetHigh(),
                                itr_DailyTrade->GetLow(),
                                itr_DailyTrade->GetClose(),
                                itr_DailyTrade->GetAdjustedClose(),
                                itr_DailyTrade->GetVolume());
                        if (ExecuteSQL(db, sql_Insert_Line) == -1)
                        {
                            cout << curr_symbol << " "
                                 << itr_DailyTrade->GetDate()
                                 << " fail to insert to PairTwoPrices table!" << endl;
                            return -1;
                        }
                        else
                        {
                            cout << curr_symbol << " " << itr_DailyTrade->GetDate() << " Successfully inserted to PairTwoPrices table!" << endl;
                        }
                    }
                }
                
                break;
            }
            case 'C': {
                break;
            }
            case 'D': {
                break;
            }
            case 'E': {
                break;
            }
            case 'F': {
                break;
            }
            case 'G': {
                break;
            }
            case 'H': {
                break;
            }
            case 'X': {
                cout << "Exiting!" << endl;
                bCompleted = true;
                break;
            }
        }
    }
    return 0;
}
