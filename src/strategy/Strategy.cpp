
#include <iostream>
#include "strategy/Strategy.h"
//#include "dataserver/SubBarsApi.h"
#include "common/Directory.h"
#include "common/XmlConfig.h"
#include "common/AppLog.h"
//#include "dataserver/RuntimeDataApiManager.h"
#include "datalib/SymbolInfoSet.h"
#include "datalib/SimpleMath.h"

Strategy::Strategy()
{
	strncpy(user_tag_, "002907", sizeof(user_tag_));

	trade_engine_ = TradeEngine::Instance();
	trade_engine_->SetSpi(this, this);	

	data_engine_ = DataEngine::Instance();
	data_engine_->SetSpi(this);
}

Strategy::~Strategy()
{
}

bool Strategy::Init(string &err){

	/*
	// �����ò���.
	XmlConfig config(Global::GetInstance()->GetConfigDir() + "config.xml");
	if (!config.Load()) 
	{
		err = "load config error";
		return false;
	}
	XmlNode node = config.FindNode("Strategy_MA");		
	std::string code = node.GetValue("code");
	symbol_ =  Symbol(PRODUCT_FUTURE, EXCHANGE_DCE, code.c_str());
	std::string period = node.GetValue("kline_period");
	s_ma_.Period = atoi(node.GetValue("short_ma").c_str());
	l_ma_.Period = atoi(node.GetValue("long_ma").c_str());
	state_ = 0;
	int jump_nums = atoi(node.GetValue("jump_nums").c_str());
	price_offset_ = SymbolInfoSet::GetInstance()->GetPriceTick(symbol_) * jump_nums;
	submit_hands_ = atoi(node.GetValue("submit_hands").c_str());
	cancel_interval_ = node.GetValue("cancel_interval");
	target_profit_value_ = atof(node.GetValue("target_profit_value").c_str());
	
	
	// ����.
	RuntimeDataApiManager* run_api_manager = new RuntimeDataApiManager(SubBarsApi::GetInstance());
	SubBarsApi::GetInstance()->SetRunApiManager(run_api_manager); //����ʵʱ����ӿ�		
	if (!run_api_manager->CreateApi(Global::GetInstance()->GetAppConfigPath(), err))	return false;

	DateTime now(NULL);
	run_api_manager->InitApi(RuntimeDataApiManager::ALL_API, now.time.hour > 18 || now.time.hour < 5);
	if (!SubBarsApi::GetInstance()->Init(Global::GetInstance()->GetAppConfigPath(), err)) 	return false;

	
	if (!SubBars(symbol_, period.c_str())) return false;
	*/
	
	symbol_ =  Symbol(PRODUCT_FUTURE, EXCHANGE_SHFE, "rb1810");
	timer_ = new TimerApi(10000, this);
	timer_->Start(3000);
	
	if (!trade_engine_->Init(Global::GetInstance()->GetConfigFile(), err)) {
		APP_LOG(LOG_LEVEL_ERROR) << err;
		return false;
	}
	if (!data_engine_->init("rb1810")) {
		return false;
	}
	
	return true;
}
/*
bool Strategy::SubBars(const Symbol& symbol, const std::string& period)
{
	HisDataParam info;
	info.symbol = symbol;
	info.count_mode = COUNT_MOD_TIME_RANGE;
	info.dim_count = atoi(period.substr(0, period.size() - 1).c_str());
	if (info.dim_count <= 0 || info.dim_count > 500)
	{
		std::cout << "��������ʧ�ܣ���Ч����:" << period;
		return false;
	}
	switch (period.at(period.size() - 1))
	{
	case 'm':case 'M':
		info.dim = DIMENSION_MINUTE;
		break;
	case 'h':case 'H':
		info.dim = DIMENSION_HOUR;
		break;
	case 'd':case 'D':
		info.dim = DIMENSION_DAY;
		break;
	default:
		std::cout << "��������ʧ�ܣ���Ч����:" << period;
		return false;
	}

	DateTime now(NULL);
	info.mode_data = now.date.year * 10000LL + now.date.month * 100LL + now.date.day;
	info.mode_data *= 100000000LL;
	if (!SubBarsApi::GetInstance()->SubBars(info, this))
	{
		cout << "SubBars error: " << SubBarsApi::GetInstance()->GetLastError() << endl;
		return false;
	}
	return true;
}

// OnTick
void Strategy::OnUpdateKline(const Bars *_bars, bool is_new){
	Bars *bars = (Bars*)_bars;

	//cout << "OnTick: " << bars->symbol.instrument << "  "  << bars->DimCount << "  " << bars->UpdateTime.Str() << "  " << bars->LastPrice << endl;

	double pre_price = trade_engine_->GetPosiPrePrice(bars->symbol);
	trade_engine_->SetPosiLastPrice(bars->symbol, bars->LastPrice);

	Locker lock(&pos_mutex_);
	for (int i=0; i < positions_.size(); ++i){
		if (positions_[i].symbol == bars->symbol){
			if (PriceUnEqual(pre_price, bars->LastPrice)) {
				PriceType pre_profit = float_profit_[i];
				if (PriceUnEqual(pre_profit, 0.)) {
					float_profit_[i] = trade_engine_->CalcFloatProfit(
						positions_[i].symbol, positions_[i].direction,
						positions_[i].open_price, bars->LastPrice, positions_[i].open_volume);
					PriceType diff_profit = float_profit_[i] - pre_profit;
					trade_engine_->UpdateAccountProfit(diff_profit);		
				}
				else {
					trade_engine_->UpdateAccountProfit(float_profit_[i], true);	
				}
			}
		}
	}
	
	std::vector<OrderData> orders;
	trade_engine_->GetValidOrder(orders);
	for (int i=0; i < orders.size(); ++i)
	{
		DateTime time = orders[i].submit_time;	
		int num = atoi(cancel_interval_.substr(0, cancel_interval_.size() - 1).c_str());
		if (cancel_interval_[1]=='m')
			time.AddMin(num);
		else if (cancel_interval_[1]=='s')
			time.AddSec(num);
		if (time < bars->UpdateTime) //˵����������1����δ�ɽ�
		{
			trade_engine_->CancelOrder(orders[i]);
			OrderParamData param;
			param.symbol = orders[i].symbol;
			param.limit_price = orders[i].direction == LONG_DIRECTION ? bars->AskPrice[0] + price_offset_ : bars->BidPrice[0] - price_offset_;
			param.volume = orders[i].total_volume - orders[i].trade_volume;
			param.order_price_flag = LIMIT_PRICE_ORDER;
			strncpy(param.user_tag, user_tag_, sizeof(UserStrategyIdType));
			param.direction = orders[i].direction;
			param.open_close_flag = orders[i].open_close_flag;
			param.hedge_flag = HF_SPECULATION;
			trade_engine_->SubmitOrder(param);
		}
	}

}
// OnBar
void Strategy::OnKlineFinish(const Bars *_bars){
	Bars *bars = (Bars*)_bars;

	s_ma_.Update(bars);
	l_ma_.Update(bars);
	cout << "s_ma_.Value[1]:"<<s_ma_.Value[1]<<" l_ma_.Value[1]:"<<l_ma_.Value[1]
	<<" s_ma_.Value[0]"<<s_ma_.Value[0]<<" l_ma_.Value[0]"<<l_ma_.Value[0]<<endl;
	switch (state_)
	{
	case 0:
		if (s_ma_.Value[1] < l_ma_.Value[1] && s_ma_.Value[0] > l_ma_.Value[0])
		{
			state_ = 1;
			trade_engine_->Buy(bars->symbol, bars->AskPrice[0] + price_offset_, submit_hands_);
			cout << "buy open\n";
		}
		else if (s_ma_.Value[1] > l_ma_.Value[1] && s_ma_.Value[0] < l_ma_.Value[0])
		{
			state_ = -1;
			trade_engine_->SellShort(bars->symbol, bars->BidPrice[0] - price_offset_, submit_hands_);
			cout << "sell open\n";
		}
		break;
	case 1:
		{
			if (s_ma_.Value[1] > l_ma_.Value[1] && s_ma_.Value[0] < l_ma_.Value[0])
			{
				PositionData long_pos;
				trade_engine_->GetLongPositionBySymbol(long_pos, bars->symbol);

				state_ = 0;
				trade_engine_->Sell(bars->symbol, bars->BidPrice[0] - price_offset_, long_pos.enable_today_volume);
				cout << "sell close\n";
			}
		}
		break;

	case -1:
		{
			if (s_ma_.Value[1] < l_ma_.Value[1] && s_ma_.Value[0] > l_ma_.Value[0])
			{
				PositionData short_pos;
				trade_engine_->GetShortPositionBySymbol(short_pos, bars->symbol);

				state_ = 0;
				trade_engine_->BuyCover(bars->symbol, bars->AskPrice[0] + price_offset_, short_pos.enable_today_volume);
				cout << "buy close\n";
			}
		}
	}

	trade_engine_->QryCtpAccount();
	cout << "OnBar:  " << bars->End[0].Str() << "  O:" << bars->Open[0] << "  H:" << bars->High[0] << "  L:" << bars->Low[0]
	<< "  C:" <<bars->Close[0] << "  V:" << bars->Volume[0] << endl;
}

// Init: OnTick & OnBar
void Strategy::OnInitKline(const Bars *_bars){
	Bars *bars = (Bars*)_bars;
	// OnTick
	s_ma_.Update(bars);
	l_ma_.Update(bars);

	trade_engine_->SetPosiLastPrice(bars->symbol, bars->LastPrice);

	// OnBar	



	cout << "Init OnTick: " << bars->symbol.instrument << "  "  << bars->DimCount << "  " << bars->UpdateTime.Str() << "  " << bars->LastPrice << endl;
	cout << "Init OnBar:  " << bars->End[0].Str() << "  O:" << bars->Open[0] << "  H:" << bars->High[0] << "  L:" << bars->Low[0]
	<< "  C:" <<bars->Close[0] << "  V:" << bars->Volume[0] << endl;
}
*/

void Strategy::UpdateAccount(){
	AccountData account;
	trade_engine_->GetAccount(account);
	cout << "�ʽ��˺ţ�\n"
		<< "��̬Ȩ��"<<account.static_balance<<" ��ӯ��"<<account.close_profit<<" ������"<<account.commision
		<< " ��̬Ȩ��"<<account.asset_balance << " ռ�ñ�֤��"<<account.margin_balance 
		<< " �µ�����"<<account.frozen_balance << " �����ʽ�"<<account.enable_balance<< endl;

	// �˻��ʽ�ĳ���趨���Զ�ȫ��ƽ�֣� �ʽ�ֹӯ���ܵ��ʽ����ﵽ�趨ֵ.
	if (account.asset_balance > target_profit_value_){
		PositionData long_pos, short_pos;
		double last_price = trade_engine_->GetPosiPrePrice(symbol_);
		if (trade_engine_->GetLongPositionBySymbol(long_pos, symbol_)){
			trade_engine_->Sell(symbol_, last_price-price_offset_, long_pos.enable_today_volume+long_pos.enable_yestd_volume);
		}
		if (trade_engine_->GetShortPositionBySymbol(short_pos, symbol_)){
			trade_engine_->BuyCover(symbol_, last_price+price_offset_, short_pos.enable_today_volume+short_pos.enable_yestd_volume);
		}
	}
}

std::string OpenCloseStr(OpenCloseFlag flag)
{
	switch (flag)
	{
	case OPEN_ORDER:
		return ("����");
	case CLOSE_ORDER:
		return ("ƽ��");
	case CLOSE_TODAY_ORDER:
		return ("ƽ��");
	case CLOSE_YESTERDAY_ORDER:
		return ("ƽ��");
	default:
		return "";
	}
}

std::string OrderStatusStr(OrderStatus status)
{
	switch(status)
	{
	case '0': return "δ֪";
	case '1': return "����";
	case '2': return "δ�ɽ�";
	case '3': return "���ֳɽ�";
	case '4': return "ȫ���ɽ�";
	case '5': return "����";
	case '6': return "�ѳ���";
	case '7': return "��δ����";
	case '8': return "�Ѵ���";
	default: return "������Ч"; 
	}
}

void Strategy::UpdateValidOrders(){
	vector<OrderData> validOrders;
	trade_engine_->GetValidOrder(validOrders);
	APP_LOG(LOG_LEVEL_INFO) << "��Ч���б�";
	for (int i=0;i<validOrders.size();++i){
		APP_LOG(LOG_LEVEL_INFO)
			<< ("ί�б��:") << validOrders[i].order_id
			<< (" ��Լ:") << validOrders[i].symbol.instrument
			<< (" ����:") << (validOrders[i].direction == LONG_DIRECTION ? "����" : "����")
			<< (" ��ƽ:") << OpenCloseStr(validOrders[i].open_close_flag)
			<< (" ί�м۸�:") << validOrders[i].limit_price
			<< (" ί������:") << validOrders[i].total_volume
			<< (" �ɽ�����:") << validOrders[i].trade_volume
			<< (" ί��ʱ��:") << validOrders[i].submit_time.time.Str()
			<< (" ״̬:") << OrderStatusStr(validOrders[i].status)
			<< (" ״̬��Ϣ:") << validOrders[i].status_msg;
	}
}
void Strategy::UpdateTrades(){
	vector<TradeData> trades;
	trade_engine_->GetAllTrade(trades);
	APP_LOG(LOG_LEVEL_INFO) <<"�ɽ����б�";
	for (int i=0;i<trades.size();++i){
		APP_LOG(LOG_LEVEL_INFO)
			<< ("��Լ") << trades[i].symbol.instrument
			<< (" ί�б��:") << trades[i].order_id
			<< (" �ɽ����:") << trades[i].trade_id
			<< (" ����:") << (trades[i].direction == LONG_DIRECTION ? "����" : "����")
			<< (" ��ƽ:") << OpenCloseStr(trades[i].open_close_flag)
			<< (" �ɽ���:") << trades[i].trade_price
			<< (" ����:") << trades[i].trade_volume
			<< (" �ɽ�ʱ��:") << trades[i].trade_time.time.Str();
	}
}
void Strategy::UpdatePositions(){
	double all_profit = 0.;//�ܸ���ӯ��.
	pos_mutex_.Lock();
	positions_.clear();
	trade_engine_->GetAllPosition(positions_);
	for (std::vector<PositionData>::const_iterator iter = positions_.begin(); iter != positions_.end(); ++iter) {
		float_profit_.push_back(iter->position_profit);
		all_profit += iter->position_profit;
	}
	pos_mutex_.Unlock();
	trade_engine_->UpdateAccountProfit(all_profit);

	vector<PositionData> positions;
	trade_engine_->GetAllPosition(positions);
	//APP_LOG(LOG_LEVEL_INFO) <<"�ֲ��б�";
	for (int i=0;i<positions.size();++i){
		APP_LOG(LOG_LEVEL_INFO)
			<< ("\t") << positions[i].symbol.instrument
			<< ("\t") << (positions[i].direction == LONG_DIRECTION ? "����" : "����")
			<< positions[i].open_volume << "��"
			//<< (" ���:") << positions[i].yestd_volume
			//<< (" ��ƽ:") << positions[i].enable_today_volume + positions[i].enable_yestd_volume
			<< ("\t�ֲ־���:") << positions[i].open_price;
	}
}
// OnOrder OnTrade
void Strategy::OnTradeEvent(TradeEventData& event){
	if (event.type == TradeEventData::ACCOUNT_EVENT) {
		//UpdateAccount();
		return;
	}
	else if (event.type == TradeEventData::ORDER_EVENT)
	{
		//UpdateValidOrders();
	}
	else if (event.type == TradeEventData::TRADE_EVENT)
	{
		//UpdateTrades();
		UpdatePositions();
	}
	else if (event.type == TradeEventData::POSITION_EVENT)
	{
		UpdatePositions();
	}

	//UpdateAccount();
}
// OnError
void Strategy::OnTradeError(const string &err){
	cout << err << endl;
}


void Strategy::OnTimer() {
	PositionData long_pos;
	trade_engine_->GetLongPositionBySymbol(long_pos, symbol_);
	static int state_;
	switch (state_)
	{
	case 0:
		if (long_pos.today_volume == 0) {
			state_ = 1;
			trade_engine_->Buy(symbol_, last_price_+10, 1);
		}
		
		break;
	case 1:
		if (long_pos.enable_today_volume > 0){
			state_ = 0;
			trade_engine_->Sell(symbol_, last_price_ - 10, long_pos.enable_today_volume);
		}
		
		break;
	}

	//trade_engine_->QryCtpAccount();
}


// OnTick
void Strategy::OnTick(FutureTick *tick){
	last_price_ = tick->last_price;
	//APP_LOG(LOG_LEVEL_INFO) << tick->symbol.Str() << "\t" << tick->last_price;

	double pre_price = trade_engine_->GetPosiPrePrice(tick->symbol);
	trade_engine_->SetPosiLastPrice(tick->symbol, tick->last_price);

	Locker lock(&pos_mutex_);
	for (int i=0; i < positions_.size(); ++i){
		if (positions_[i].symbol == tick->symbol){
			if (PriceUnEqual(pre_price, tick->last_price)) {
				PriceType pre_profit = float_profit_[i];
				if (PriceUnEqual(pre_profit, 0.)) {
					float_profit_[i] = trade_engine_->CalcFloatProfit(
						positions_[i].symbol, positions_[i].direction,
						positions_[i].open_price, tick->last_price, positions_[i].open_volume);
					PriceType diff_profit = float_profit_[i] - pre_profit;
					trade_engine_->UpdateAccountProfit(diff_profit);		
				}
				else {
					trade_engine_->UpdateAccountProfit(float_profit_[i], true);	
				}
			}
		}
	}

	/*
	std::vector<OrderData> orders;
	trade_engine_->GetValidOrder(orders);
	for (int i=0; i < orders.size(); ++i)
	{
		DateTime time = orders[i].submit_time;	
		int num = atoi(cancel_interval_.substr(0, cancel_interval_.size() - 1).c_str());
		if (cancel_interval_[1]=='m')
			time.AddMin(num);
		else if (cancel_interval_[1]=='s')
			time.AddSec(num);
		if (time < tick->date_time) //˵����������1����δ�ɽ�
		{
			trade_engine_->CancelOrder(orders[i]);
			OrderParamData param;
			param.symbol = orders[i].symbol;
			param.limit_price = orders[i].direction == LONG_DIRECTION ? tick->ask_price + price_offset_ : tick->bid_price - price_offset_;
			param.volume = orders[i].total_volume - orders[i].trade_volume;
			param.order_price_flag = LIMIT_PRICE_ORDER;
			strncpy(param.user_tag, user_tag_, sizeof(UserStrategyIdType));
			param.direction = orders[i].direction;
			param.open_close_flag = orders[i].open_close_flag;
			param.hedge_flag = HF_SPECULATION;
			trade_engine_->SubmitOrder(param);
		}
	}
	*/
}


void Strategy::OnKline(Kline *kline) {

}