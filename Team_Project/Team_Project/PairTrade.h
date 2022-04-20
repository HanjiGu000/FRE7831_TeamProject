//
//  PairTrade.hpp
//  Team_Project
//
//  Created by Hanji Gu on 4/19/22.
//

#ifndef PairTrade_hpp
#define PairTrade_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;


class TradeData
{
private:
    string sDate;
    double dOpen;
    double dHigh;
    double dLow;
    double dClose;
    double dAdjClose;
    long lVolume;
public:
    TradeData(): sDate(""), dOpen(0), dClose(0),
                 dHigh(0), dLow(0), dAdjClose(0), lVolume(0) {}
    TradeData(string sDate_, double dOpen_, double dHigh_,
              double dLow_, double dClose_, double dAdjClose_, long lVolume_):
                sDate(sDate_), dOpen(dOpen_), dHigh(dHigh_),
                dLow(dLow_), dClose(dClose_), dAdjClose(dAdjClose_), lVolume(lVolume_) {}
    TradeData(const TradeData & TradeData):
                sDate(TradeData.sDate), dOpen(TradeData.dOpen), dHigh(TradeData.dHigh),
                dLow(TradeData.dLow), dClose(TradeData.dClose), dAdjClose(TradeData.dAdjClose),
                lVolume(TradeData.lVolume) {}
    TradeData operator=(const TradeData & TradeData)
    {
        sDate = TradeData.sDate;
        dOpen = TradeData.dOpen;
        dHigh = TradeData.dHigh;
        dLow = TradeData.dLow;
        dClose = TradeData.dClose;
        dAdjClose = TradeData.dAdjClose;
        lVolume = TradeData.lVolume;
        return *this;
    }
};


class Stock
{
private:
    string sSymbol;
    vector<TradeData> trades;
public:
    Stock(): sSymbol("") {}
    Stock(string sSymbol_, const vector<TradeData> trades_):
            sSymbol(sSymbol_), trades(trades_) {}
    Stock(const Stock & stock):
            sSymbol(stock.sSymbol), trades(stock.trades) {}
    Stock operator=(const Stock & stock)
    {
        sSymbol = stock.sSymbol;
        trades = stock.trades;
        return *this;
    }
    
    void addTrade(const TradeData & trade)
    {
        trades.push_back(trade);
    }
    string getSymbol()
    {
        return sSymbol;
    }
    const vector<TradeData> & getTrades() const
    {
        return trades;
    }
    
    friend ostream & operator<<(ostream & ostr, Stock & stock)
    {
        ostr << "Symbol: " << stock.sSymbol << endl;
        for (vector<TradeData>::const_iterator itr = stock.trades.begin(); itr != stock.trades.end(); itr ++)
        {
            ostr << *itr;
        }
        return ostr;
    }

};


#endif /* PairTrade_hpp */
