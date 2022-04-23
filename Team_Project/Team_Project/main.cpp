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

int main(int argc, const char * argv[]) {

    sqlite3* db = NULL;
    string database = "PairTrading.db";
    string trading_pair = "PairTrading.txt";
    vector<StockPairPrices> pairs_vec;
    
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
                    sprintf(sql_Insert_Line, "INSERT INTO StockPairs(id, Symbol1, Symbol2, Volatility, profit_loss) VALUES(%d, \"%s\", \"%s\", %f, %f)",
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
                }
                break;
            }
            case 'B': {
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
