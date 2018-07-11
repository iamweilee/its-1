#ifndef ITS_TRADE_EIGINE_H_
#define ITS_TRADE_EIGINE_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include "common/SpinLock.h"
#include "account/CTPMarginCommision.h"

//namespace itstation {


// �����¼�.
struct TradeEventData
{
	enum EventType
	{
		ORDER_EVENT,
		TRADE_EVENT,
		POSITION_EVENT,
		ACCOUNT_EVENT
	};
	UserStrategyIdType flag;
	EventType type;
	void* data;
};
class TradeEventSpi 
{
public:
	virtual void OnTradeEvent(TradeEventData& event) = 0;
};
// ���׳�����ʾ.
class TradeErrorSpi 
{
public:
	virtual void OnTradeError(const std::string &) = 0;
};

class ACCOUNT_API TradeEngine : public TradeSpi, public CTPMarginCommision
{
public:
	static TradeEngine* Instance();
	void SetSpi(TradeEventSpi* trade_spi, TradeErrorSpi *err_spi=NULL);
	bool Init(std::string& err);
	void Denit();
	
	///���׽ӿڣ����ر���ί�б��LocalOrderID>0.�����������0
	// ��(LimitPrince)
	int Buy(const Symbol& symbol, double price, int volume, const UserStrategyIdType user_tag = NULL);
	// ��(MarketPrince).
	int BuyMar(const Symbol& symbol, int volume, const UserStrategyIdType user_tag = NULL);	
	// ��ƽ. 
	int BuyCover(const Symbol& symbol, double price, int volume, bool is_td/*�Ƿ���*/, const UserStrategyIdType user_tag = NULL);
	// ��ƽ. ����ƽ��֣���ƽ���.
	int BuyCover(const Symbol& symbol, double price, int volume, const UserStrategyIdType user_tag = NULL);	
	// ����(LimitPrince)
	int SellShort(const Symbol& symbol, double price, int volume, const UserStrategyIdType user_tag = NULL);
	// ����(MarketPrince)
	int SellShortMar(const Symbol& symbol,int volume, const UserStrategyIdType user_tag = NULL);
	// ��ƽ
	int Sell(const Symbol& symbol, double price, int volume, bool is_td/*�Ƿ���*/, const UserStrategyIdType user_tag = NULL);
	// ��ƽ. ����ƽ��֣���ƽ���.
	int Sell(const Symbol& symbol, double price, int volume, const UserStrategyIdType user_tag = NULL);		
	// ƽ�֣���ƽ��֣���ƽ���.
	
	// ���ݱ���ί�б�ų���.
	void CancelOrder(int LocalOrderID);
	// ���ݺ�Լ������.
	void CancelOrder(const Symbol& sym, const UserStrategyIdType user_tag = NULL);

	///�������׽ӿڣ�
	// ���ر���ί�б��LocalOrderID>0.�����������0
	int SubmitOrder(const OrderParamData& param);
	// ����.
	void CancelOrder(const OrderData& param);	

	///��ѯ�ӿ�.
	// ���ʽ��˻�.
	void GetAccount(AccountData &);
	// ��ί��.
	void GetAllOrder(std::vector<OrderData>& orders, const UserStrategyIdType user_tag = NULL);
	void GetValidOrder(std::vector<OrderData>& orders, const UserStrategyIdType user_tag = NULL);//�ɳ���.
	bool GetOrderByLocalId(OrderData& order, NumberIdType local_order_id, const UserStrategyIdType user_tag = NULL);//���ݱ��ر��.
	void GetOrderBySymbol(std::vector<OrderData>& orders, const Symbol& sym, const UserStrategyIdType user_tag = NULL);
	// ��ɽ�.
	void GetAllTrade(std::vector<TradeData>& trades, const UserStrategyIdType user_tag = NULL);
	void GetTradeBySymbol(std::vector<TradeData>& trades, const Symbol& sym, const UserStrategyIdType user_tag = NULL);
	// ��ֲ� (ע�⣺�ֲֵ����¼ۡ��ֲ�ӯ�����ֲ���ֵ����ʵʱ���£������û�ȡʱ�ټ���).
	void GetAllLongPosition(std::vector<PositionData>& pos, const UserStrategyIdType user_tag = NULL);
	void GetAllShortPosition(std::vector<PositionData>& pos, const UserStrategyIdType user_tag = NULL);
	void GetAllPosition(std::vector<PositionData>& pos, const UserStrategyIdType user_tag = NULL);
	// user_tag���ΪNULL����long_positions_�ĵ�һ��.
	bool GetLongPositionBySymbol(PositionData &pos, const Symbol& sym, const UserStrategyIdType user_tag = NULL);
	bool GetShortPositionBySymbol(PositionData &pos, const Symbol& sym, const UserStrategyIdType user_tag = NULL);

	///tick�������������仯����Ҫ���³ֲֵ����¼ۡ�����ӯ��.
	// ������仯ʱ�������˻����ܸ���ӯ��.
	void UpdateAccountProfit(PriceType &profit, bool init = false);
	PriceType CalcFloatProfit(const Symbol &, OrderDirection, PriceType open_price, PriceType last_price, VolumeType open_volume);
	// �����гֲֵĺ�Լ�����¼ۡ�ע�⣺���ǵ�Ч�ʣ��ֲֵ����¼ۡ��ֲ�ӯ�����ֲ���ֵ����ʵʱ���£������û�ȡʱ�ټ���.
	void SetPosiLastPrice(const Symbol &, PriceType);
	PriceType GetPosiPrePrice(const Symbol &sym);

	virtual ~TradeEngine();
	// �첽��ѯCTP���ʽ��˺�.
	void QryCtpAccount();
private:
	TradeEngine();

	virtual void OnError(const int request_id, const std::string& error_msg);
	virtual void OnOrderError(OrderData* order_data);
	virtual void OnDisconnect(const std::string& reson);
	virtual void OnConnect();
	virtual void OnOrder(OrderData* order_data);
	virtual void OnTrade(TradeData* trade_data);
	virtual void OnCancelOrder(OrderData* canceled_data);
	virtual void OnQryOrder(int req_id, OrderData* order_data, const std::string& err, bool is_last);
	virtual void OnQryTrade(int req_id, TradeData* trade_data, const std::string& err, bool is_last);
	virtual void OnQryAccount(int req_id, AccountData* trade_data, const std::string& err, bool is_last);
	virtual void OnQryPosition(int req_id, PositionData* trade_data, const std::string& err, bool is_last);
	
	void OnOrder(OrderData *order_data, bool is_qry, bool is_last = false);

	///���ݳɽ���Ϣʵʱ���³ֲ�.
	void UpdatePoswWithTrade(TradeData* trade_data);
	void OpenLong(TradeData* trade_data);
	void CloseShort(TradeData* trade_data);
	void OpenShort(TradeData* trade_data);
	void CloseLong(TradeData* trade_data);

	///�ж��ǲ�����Ч����  û�г�������ʧ��.
	bool IsOrderValid(OrderStatus status);	
	///���û���ֲ�ʱ������ֲֵ����¼ۡ��ֲ�ӯ�����ֲ���ֵ.
	void CalcPosiInfoByLastPrice(PositionData &);
	///ˢ�½���
	void SendTradeEventData(TradeEventData::EventType, const UserStrategyIdType, void *);


	///---------------�����ʽ��˻�----------------------------------------------
	///����ʱ�����ᱣ֤��+���������� = �µ�����.
	void FrozenAccountBalance(const OrderData *order_data);
	///�������ֵ�ʱ��ȡ���µ�����.
	void CancelFrozenAccountBalance(const OrderData *order_data);
	///�н������֡�ƽ��ʱ����ʹ���ɽ�Ҳ��1��������.
	void MinusAccountCommision(const OrderData *order_data);
	///�ɽ�ʱ�۳�������.
	void MinusAccountCommision(TradeData *trade_data);
	///����ռ�ñ�֤��.
	void MarginAccountBalance(const TradeData *trade_data);
	///ƽ��ȡ��ռ��.
	void CancelMarginAccountBalance(const TradeData *trade_data);	
	///ƽ��ʱ����ƽ��ӯ��.
	void UpdateAccountCloseProfit(const TradeData *trade_data);


private:
	TradeEventSpi* event_spi_;
	TradeErrorSpi* err_spi_;
	static TradeEngine* self_;
	bool is_init_;

	///�ʽ��˻� ��1���µ�ʱ���ᱣ֤�𣬳���ʱȡ������ 2���ɽ�ʱ�۳������� 3������ռ�ñ�֤��ƽ��ȡ��ռ�� 4��ƽ��ʱ����ƽ��ӯ��.
	AccountData account_data_;
	std::map<NumberIdType, OrderData> valid_orders_;
	std::map<NumberIdType, OrderData> all_orders_;
	std::list<OrderData> rejected_orders_;
	std::vector<TradeData> trades_;
	std::map<std::string, std::map<Symbol, PositionData> > long_positions_; // ����user_tag���з���.
	std::map<std::string, std::map<Symbol, PositionData> > short_positions_;
	///�ֲֺ�Լ���ּۡ��ֲֵ����¼ۡ��ֲ�ӯ�����ֲ���ֵ����ʵʱ���£������û�ȡʱ�ټ���.
	std::map<Symbol, PriceType> last_prices_;

	SpinLock account_mutex_;
	SpinLock order_mutex_;
	SpinLock trade_mutex_;
	SpinLock pos_mutex_;
	SpinLock last_price_mutex_;

	bool re_qry_pos_; // initʱ���������ontrade����Ҫ���²�ֲ�.
	
};


#endif