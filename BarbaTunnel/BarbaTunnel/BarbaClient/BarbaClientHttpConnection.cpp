#include "StdAfx.h"
#include "BarbaClientHttpConnection.h"
#include "BarbaClientApp.h"


BarbaClientHttpConnection::BarbaClientHttpConnection(BarbaClientConfig* config, BarbaClientConfigItem* configItem, u_short tunnelPort)
	: BarbaClientConnection(config, configItem)
	, Courier()
{
	this->SessionId = BarbaUtils::GetRandom(100000, UINT_MAX-1);
	this->TunnelPort = tunnelPort;

	BarbaCourierCreateStrcut cs = {0};
	cs.FakeFileHeaderSizeKeyName = configItem->FakeFileHeaderSizeKeyName.data();
	cs.FakeFileMaxSize = configItem->FakeFileMaxSize;
	cs.SessionKeyName = configItem->SessionKeyName.data();
	cs.FakeHttpGetTemplate = theClientApp->FakeHttpGetTemplate.data();
	cs.FakeHttpPostTemplate = theClientApp->FakeHttpPostTemplate.data();
	cs.MaxConnenction = configItem->MaxUserConnections;
	cs.SessionId = this->SessionId;
	cs.ThreadsStackSize = BARBA_SocketThreadStackSize;
	this->Courier = new BarbaClientHttpCourier(&cs, config->ServerIp, tunnelPort, this);
}

BarbaClientHttpConnection::~BarbaClientHttpConnection(void)
{
	theApp->AddThread(this->Courier->Delete());
}

bool BarbaClientHttpConnection::ShouldProcessPacket(PacketHelper* packet)
{
	//just process outgoing packets
	return packet->GetDesIp()==GetServerIp() && ConfigItem->ShouldGrabPacket(packet);
}

bool BarbaClientHttpConnection::ProcessPacket(PacketHelper* packet, bool send)
{
	if (send)
	{
		this->SetWorkingState(packet->GetIpLen(), true);
		this->Courier->SendPacket(packet);
	}
	else
	{
		this->SendPacketToMstcp(packet);
	}
	return true;
}