


#include "mongoose.h"  
#include <string>
#include <format>

std::string LOG{"Nil"};

extern int COT_MULTICAST_SEND_PORT;
extern char COT_MULTICAST_SEND_GROUP[20];

extern std::string COM_Port;
extern UINT baud;

extern int totalAISRxCount;
extern int totalCoTTx;


std::string GetIniStatus();
int GetNumTracks();


//based on webUI push
//https://mongoose.ws/documentation/tutorials/webui/webui-push/


//public function
void SetupMongoose();

struct mg_mgr mgr;  // Event manager

//private functions
static void timer_fn(void* arg);
static void push(struct mg_mgr* mgr, const char* name, const void* data);
static void event_handler(struct mg_connection* c, int ev, void* ev_data);

static const char* s_listen_on = "http://localhost:8500";
//static const char* s_web_root = "./web_root/";
static const char* s_web_root = "c:/web_root/";   //need absolute path for Windows Service

static int connCount = 0;

void SetupMongoose()
{
    mg_log_set(2);   // Set to 3 to enable debug
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_timer_add(&mgr, 2000, MG_TIMER_REPEAT, timer_fn, &mgr);
    mg_http_listen(&mgr, s_listen_on, event_handler, NULL);  // Create HTTP listener

    for (;;) mg_mgr_poll(&mgr, 500);              // Infinite event loop
    mg_mgr_free(&mgr);                            // Free manager resources
}


void SetupMongooseAsService()
{
    mg_log_set(2);   // Set to 3 to enable debug
    mg_mgr_init(&mgr);  // Initialise event manager
    mg_timer_add(&mgr, 1000, MG_TIMER_REPEAT, timer_fn, &mgr);
    mg_http_listen(&mgr, s_listen_on, event_handler, NULL);  // Create HTTP listener
}



static void event_handler(struct mg_connection* c, int ev, void* ev_data)
{
	if (ev == MG_EV_WS_OPEN)
	{
		connCount++;
	}
	if (ev == MG_EV_CLOSE)
	{
		connCount--;
	}

	if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message* hm = (struct mg_http_message*)ev_data;
		if (mg_match(hm->uri, mg_str("/api/watch"), NULL))
		{
			mg_ws_upgrade(c, hm, NULL);  // Upgrade HTTP to Websocket
			c->data[0] = 'W';           // Set some unique mark on the connection
		}
		else
		{
			struct mg_http_serve_opts opts {};
			opts.root_dir = "c:/web_root/";
			opts.ssi_pattern = 0;    // SSI file name pattern, e.g. #.shtml
			opts.extra_headers = 0;  // Extra HTTP headers to add in responses
			opts.mime_types = 0;     // Extra mime types, ext1=type1,ext2=type2,..
			opts.page404 = 0;        // Path to the 404 page, or NULL by default
			opts.fs = 0;
			mg_http_serve_dir(c, (mg_http_message*)ev_data, &opts);
		}
	}
}




// Push to all watchers
static void push(struct mg_mgr* mgr, const char* name, const void* data)
{
	struct mg_connection* c;
	for (c = mgr->conns; c != NULL; c = c->next)
	{
		if (c->data[0] != 'W') continue;
		mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{%m:%m, %m:%m,%m:%m}", MG_ESC("type"), MG_ESC("ais"), MG_ESC("Event"), MG_ESC(name), MG_ESC("Data"), MG_ESC(data));
	}
}


// Push to all watchers
static void pushIniData(struct mg_mgr* mgr)
{
	struct mg_connection* c;
	for (c = mgr->conns; c != NULL; c = c->next)
	{
		if (c->data[0] != 'W') continue;
		std::string b = std::format("{}", baud);
		std::string p = std::format("{}", COT_MULTICAST_SEND_PORT);
		mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{%m:%m,%m:%m, %m:%m,%m:%m,%m:%m,%m:%m}", MG_ESC("type"), MG_ESC("ini"), MG_ESC("Version"),  MG_ESC(__DATE__), MG_ESC("Port"), MG_ESC(COM_Port.c_str()), MG_ESC("Baud"), MG_ESC(b.c_str()), MG_ESC("CoTIP"), MG_ESC(COT_MULTICAST_SEND_GROUP), MG_ESC("CoTPort"), MG_ESC(p.c_str()));

	}
}



// Push to all watchers
static void pushAISStatus(struct mg_mgr* mgr)
{
	static int hb = 0;
	struct mg_connection* c;
	for (c = mgr->conns; c != NULL; c = c->next)
	{
		if (c->data[0] != 'W') continue;
		std::string b = std::format("{}", hb++);
		std::string a = std::format("{}", totalAISRxCount);
		std::string cot = std::format("{}", totalCoTTx);
		mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{%m:%m, %m:%m,%m:%m,%m:%m}", MG_ESC("type"), MG_ESC("ais"), MG_ESC("HeartBeat"), MG_ESC(b.c_str()), MG_ESC("AISRxMsgCount"), MG_ESC(a.c_str()), MG_ESC("CoTTxMsgCount"), MG_ESC(cot.c_str()));
	}
}


// Push to all watchers
static void pushLog(struct mg_mgr* mgr)
{
	static int hb = 0;
	struct mg_connection* c;
	for (c = mgr->conns; c != NULL; c = c->next)
	{
		if (c->data[0] != 'W') continue;
		
		std::string trackListSize = LOG + std::format(" #Trks: {}", GetNumTracks());

		mg_ws_printf(c, WEBSOCKET_OP_TEXT, "{%m:%m, %m:%m}", MG_ESC("type"), MG_ESC("log"), MG_ESC("Log"), MG_ESC(trackListSize.c_str())  );
	}
}




static void timer_fn(void* arg)
{
	if (0 == connCount) return;

	struct mg_mgr* mgr = (struct mg_mgr*)arg;
	
	pushIniData(mgr);
	pushAISStatus(mgr);
	pushLog(mgr);

}



