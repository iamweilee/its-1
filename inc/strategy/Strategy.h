#ifndef STRATEGY_H
#define STRATEGY_H

//#include "dataserver/DataApiDefine.h"
#include "account/TradeEngine.h"
//#include "account/DataEngine.h"
#include "strategy/Client.h"
#include "network/Timer.h"
//#include "ctp/MarketDataApi.h"
//#include "IndexLib/iMA.h"


using namespace std;

class STRATEGY_API Strategy : public TradeEventSpi, public TradeErrorSpi, public TimerSpi, public ClientSpi//, public DataEventSpi
{
public:
	Strategy();
	virtual ~Strategy();
	
	bool Init(string &);
	void Denit();

private:
	/*
	// OnTick
	virtual void OnUpdateKline(const Bars *bars, bool is_new);
	// OnBar
	virtual void OnKlineFinish(const Bars *bars);
	// Init: OnTick & OnBar
	virtual void OnInitKline(const Bars *bars);

	bool SubBars(const Symbol& symbol, const std::string& period);
	*/

	virtual void OnTimer();
	virtual void OnData(Bars *, bool);
	virtual void OnError(const string &);

	// OnOrder OnTrade
	virtual void OnTradeEvent(TradeEventData& event);
	// OnError
	virtual void OnTradeError(const string &);
	
	//virtual void OnTick(FutureTick *);

	void UpdateAccount();
	void UpdateValidOrders();
	void UpdateTrades();
	void UpdatePositions();

	// ָ��.
	// iMA s_ma_; // 5min
	// iMA l_ma_; // 10min

	// ���Ա��.
	UserStrategyIdType user_tag_;
	std::vector<PositionData> positions_;
	std::vector<PriceType> float_profit_; // �ֲֵĸ���ӯ��.

	SpinLock pos_mutex_;
	int state_; //0:û���֡�1:����֡�2:���ղ�.
	double price_offset_; //���ּ�+N
	int submit_hands_;//�µ�����.
	string cancel_interval_;//�������ʱ��.

	// �˻��ʽ�ĳ���趨���Զ�ȫ��ƽ�֣� �ʽ�ֹӯ���ܵ��ʽ����ﵽ�趨ֵ.
	double target_profit_value_;
	Symbol symbol_;
	double last_price_;

	TradeEngine *trade_engine_;
	//DataEngine *data_engine_;

	TimerApi *timer_;
	Client *client_;
};

#endif