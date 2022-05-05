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
#include <cmath>

int main(int argc, const char * argv[]) {

    sqlite3* db = NULL;
    string database = "PairTrading.db";
    string trading_pair = "PairTrading.txt";
    vector<StockPairPrices> pairs_vec;
    vector<string> pair_one_vec;
    vector<string> pair_two_vec;
    map<string, Stock> stockMapOne;
    map<string, Stock> stockMapTwo;
    double K_ = 1;
    vector<pair<string, string>> trading_pairs = GetPairs(trading_pair);

    
    if (OpenDatabase(database.c_str(), db) != 0)
        return -1;
    
    bool bCompleted = false;
    char selection;

    while (!bCompleted)
    {
        std::cout << endl;
        std::cout << "Menu" << endl;
        std::cout << "================" << endl;
        std::cout << "A - Create and Populate Pair Table" << endl;
        std::cout << "B - Retrieve and Populate Historical Data for Each Stock" << endl;
        std::cout << "C - Create PairPrices Table" << endl;
        std::cout << "D - Calculate Volatility" << endl;
        std::cout << "E - Back Test" << endl;
        std::cout << "F - Calculate Profit and Loss for Each Pair" << endl;
        std::cout << "G - Manual Testing" << endl;
        std::cout << "H - Drop All the Tables" << endl;
        std::cout << "I - Set K (Default 1)" << endl;
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
                                                + "symbol1 CHAR(20) NOT NULL,"
                                                + "symbol2 CHAR(20) NOT NULL,"
                                                + "volatility FLOAT NOT NULL,"
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
                
                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM StockPairs;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
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
//                string end_date = config_map["end_date"];
                string end_date = GetCurrentDate();
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
                
                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM PairOnePrices;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                
                cout << "===================================================================" << endl;
                sql_Select = string("SELECT * FROM PairTWOPrices;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                break;
            }

            case 'C': { // Create PairPrices Table --- sql statement
                string sql_DropTable = "DROP TABLE IF EXISTS PairPrices";
                if (DropTable(db, sql_DropTable.c_str()) == -1)
                    return -1;
                
                string sql_CreateTable = string("CREATE TABLE IF NOT EXISTS PairPrices")
                    + "(symbol1 CHAR(20) NOT NULL,"
                    + "symbol2 CHAR(20) NOT NULL,"
                    + "date CHAR(20) NOT NULL,"
                    + "open1 REAL NOT NULL,"
                    + "close1 REAL NOT NULL,"
                    + "adjusted_close1 REAL NOT NULL,"
                    + "open2 REAL NOT NULL,"
                    + "close2 REAL NOT NULL,"
                    + "adjusted_close2 REAL NOT NULL,"
                    + "profit_loss FLOAT NOT NULL,"
                    + "PRIMARY KEY(symbol1, symbol2, date)"
//                    + ", Foreign Key(symbol1, symbol2, date) references (PairOnePrices(symbol), PairTwoPrices(symbol, date))\n"
                    + ");";
                
                if (ExecuteSQL(db, sql_CreateTable.c_str()) == -1)
                    return -1;
                cout << "Created PairPrices table" << endl;
                
                
                string sql_Insert = string("Insert into PairPrices ")
                    + "SELECT StockPairs.symbol1 AS symbol1, "
                            + "StockPairs.symbol2 AS symbol2, "
                            + "PairOnePrices.date AS date, "
                            + "PairOnePrices.open AS open1, PairOnePrices.close AS close1, "
                            + "PairOnePrices.adjusted_close AS adjusted_close1, "
                            + "PairTwoPrices.open AS open2, PairTwoPrices.close AS close2, "
                            + "PairTwoPrices.adjusted_close AS adjusted_close2, "
                            + "0.0 AS profit_loss "
                    + "FROM StockPairs, PairOnePrices, PairTwoPrices "
                    + "WHERE ((StockPairs.symbol1=PairOnePrices.symbol) AND "
                            + "(StockPairs.symbol2=PairTwoPrices.symbol) AND "
                            + "(PairOnePrices.date=PairTwoPrices.date)) "
                    + "ORDER By symbol1, symbol2;";
                
                if (ExecuteSQL(db, sql_Insert.c_str()) == -1)
                    return -1;
                
                cout << "Inserted data into PairPrices table" << endl;
                
                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM PairPrices;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                break;
            }
                
            case 'D': { // calculate volatility
                string back_test_start_date = "2022-01-01";
                
                string calculate_volatility_for_pair =  string("Update StockPairs SET volatility = ")
                    + "(SELECT(AVG((adjusted_close1/adjusted_close2) * (adjusted_close1/adjusted_close2)) "
                    + "- AVG(adjusted_close1/adjusted_close2) * AVG(adjusted_close1/adjusted_close2)) as variance "
                    + "FROM PairPrices "
                    + "WHERE StockPairs.symbol1 = PairPrices.symbol1 AND StockPairs.symbol2 = PairPrices.symbol2 AND "
                + "PairPrices.date <= \'back_test_start_date\');";
                
                if (ExecuteSQL(db, calculate_volatility_for_pair.c_str()) == -1)
                    return -1;
                cout << "Calculated volatility for PairPrices table" << endl;
                
                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM StockPairs;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                break;
            }

            case 'E': {
                string sql_Update = string("UPDATE PairPrices ")
                    + "SET profit_loss = tttt.position "
                    + "FROM "
                        + "(SELECT IIF(ABS((Close1d1/Close2d1) - (Open1d2/Open2d2)) >= " + to_string(K_) + "*volatility, -10000*(Close1d2-Open1d2)+10000*(Open1d2/Open2d2)*(Close2d2-Open2d2), 10000*(Close1d2-Open1d2)-10000*(Open1d2/Open2d2)*(Close2d2-Open2d2)) AS position, "
                                + "symbol1 AS Symbol1, symbol2 AS Symbol2, date AS date "
                        + "FROM "
                            + "(SELECT pp.symbol1 AS symbol1, pp.symbol2 AS symbol2, "
                                     + "pp.close1 AS Close1d2, pp.close2 AS Close2d2, "
                                     + "pp.open1 AS Open1d2, pp.open2 AS Open2d2, "
                                     + "Lag(pp.close1, 1) OVER() AS Close1d1, Lag(pp.close2, 1) OVER() AS Close2d1, "
                                     + "sp.volatility AS volatility, "
                                     + "pp.date "
                              + "FROM PairPrices pp, StockPairs sp "
                              + "WHERE pp.date >= \"2022-01-01\" "
                              + "AND pp.symbol1 = sp.symbol1 "
                              + "AND pp.symbol2 = sp.symbol2) "
//                        + "WHERE date <= \"2022-04-21\" "
                        + "WHERE date > \"2022-01-03\") tttt "
                    + "WHERE "
                        + "(PairPrices.symbol1 = tttt.Symbol1 "
                        + "AND PairPrices.symbol2 = tttt.Symbol2 "
                        + "AND PairPrices.date = tttt.date)";
                
                if (ExecuteSQL(db, sql_Update.c_str()) == -1)
                    return -1;
                
                cout << "Calculated daily profit and loss for back testing. " << endl;

                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM PairPrices WHERE profit_loss != 0;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                break;
                
            }

            case 'F': {
                string sql_Update = string("UPDATE StockPairs ")
                                         + "SET profit_loss = yyy.pnl "
                                         + "FROM "
                                            + "(SELECT SUM(pp.profit_loss) AS pnl, pp.symbol1 AS symbol1, pp.symbol2 AS symbol2 "
                                            + "FROM PairPrices pp "
                                            + "WHERE pp.date >= '2022' "
                                            + "GROUP BY pp.symbol1, pp.symbol2) yyy "
                                         + "WHERE "
                                            + "(yyy.symbol1 = StockPairs.symbol1 AND yyy.symbol2 = StockPairs.symbol2)";
                
                if (ExecuteSQL(db, sql_Update.c_str()) == -1)
                    return -1;
                
                cout << "Calculated aggregated PNL to StockPairs. " << endl;
                
                cout << "===================================================================" << endl;
                string sql_Select = string("SELECT * FROM StockPairs;");
                if (ShowTable(db, sql_Select.c_str()) != 0)
                {
                    return -1;
                }
                break;
            }

            case 'G': {
                string symbol_1, symbol_2;
                double Close1_Day1, Close2_Day1, Open1_Day2, Open2_Day2, Close1_Day2, Close2_Day2, volatility, pnl;
                
                cout << "Enter Pair Selection" << endl;
                cout << "Symbol_1: ";
                cin >> symbol_1;
                transform(symbol_1.begin(), symbol_1.end(), symbol_1.begin(), ::toupper);
                
                
                cout << "Symbol_2: ";
                cin >> symbol_2;
                cout << endl;
                transform(symbol_2.begin(), symbol_2.end(), symbol_2.begin(), ::toupper);
                
                if (find(trading_pairs.begin(), trading_pairs.end(), pair<string, string>(symbol_1, symbol_2)) == trading_pairs.end()
                    && find(trading_pairs.begin(), trading_pairs.end(), pair<string, string>(symbol_2, symbol_1)) == trading_pairs.end())
                {
                    cout << "Invaild Traiding Pair, Please Check!" << endl;
                    break;
                }
                
                string sql_get = "SELECT volatility FROM StockPairs WHERE symbol1 = \"" + symbol_1 + "\" AND symbol2 = \"" + symbol_2 + "\";";
                if (GetSingleVolatility(db, sql_get.c_str()) != -1.0)
                {
                    volatility = GetSingleVolatility(db, sql_get.c_str());
                }
                else
                {
                    cout << "Inversed Pair, Please change the order of symbol_1 and symbol_2." << endl;
                    break;
                }
                
                cout << symbol_1 + " and " + symbol_2 + " volatility is " << volatility << endl;
                
                cout << "Close1_Day1: ";
                cin >> Close1_Day1;
                
                cout << "Close2_Day1: ";
                cin >> Close2_Day1;
                cout << endl;
                
                cout << "Close1_Day2: ";
                cin >> Close1_Day2;
                
                cout << "Close2_Day2: ";
                cin >> Close2_Day2;
                cout << endl;
                
                cout << "Open1_Day2: ";
                cin >> Open1_Day2;
                
                cout << "Open2_Day2: ";
                cin >> Open2_Day2;
                cout << endl;
                
                cout << "K: ";
                cin >> K_;
                cout << endl;
                
                if (abs((Close1_Day1/Close2_Day1) - (Open1_Day2/Open2_Day2)) >= K_*volatility)
                {
                    pnl = -10000*(Close1_Day2 - Open1_Day2)
                            + 10000*(Open1_Day2/Open2_Day2)*(Close2_Day2-Open2_Day2);
                }
                else
                {
                    pnl = 10000*(Close1_Day2 - Open1_Day2)
                        - 10000*(Open1_Day2/Open2_Day2)*(Close2_Day2-Open2_Day2);
                }
                
                cout << "Expected PNL: " << pnl << endl;
                
                K_ = 1;
                break;
            }

            case 'H': {
                string sql_DropTable = "DROP TABLE IF EXISTS PairOnePrices";
                if (DropTable(db, sql_DropTable.c_str()) == -1)
                    return -1;
                cout << "PairOnePrices Table Dropped" << endl;
                
                sql_DropTable = "DROP TABLE IF EXISTS PairTwoPrices";
                if (DropTable(db, sql_DropTable.c_str()) == -1)
                    return -1;
                cout << "PairTwoPrices Table Dropped" << endl;
                
                sql_DropTable = "DROP TABLE IF EXISTS PairPrices";
                if (DropTable(db, sql_DropTable.c_str()) == -1)
                    return -1;
                cout << "PairPrices Table Dropped" << endl;
                
                sql_DropTable = "DROP TABLE IF EXISTS StockPairs";
                if (DropTable(db, sql_DropTable.c_str()) == -1)
                    return -1;
                cout << "StockPairs Table Dropped" << endl;

                break;
            }

            case 'I': {
                cout << "Current K: " << K_ << endl;
                cout << "User choice of K: ";
                cin >> K_;
                cout << "New K_: " << K_ << endl;
                break;
            }
                
            case 'X': {
                cout << "Exiting!" << endl;
                bCompleted = true;
                break;
            }
        }
    }
    CloseDatabase(db);
    return 0;
}
