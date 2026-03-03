

#include "mongoose.h"
#include "ServiceBase.h"


extern struct mg_mgr mgr;  // Declare event manager
void SetupMongoose();
void SetupMongooseAsService();


void MongooseServiceInitialize()
{
	SetupMongooseAsService();
}


void MongooseServiceMainLoop()
{
	mg_mgr_poll(&mgr, 1000);
}

void MongooseServiceShutdown()
{
	mg_mgr_free(&mgr);
}

