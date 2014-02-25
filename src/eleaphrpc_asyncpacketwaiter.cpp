#include "eleaphrpc_asyncpacketwaiter.h"

EleaphRpcAsyncPacketWaiter::EleaphRpcAsyncPacketWaiter(EleaphRpc *eleaphProto, QString strMethod)
{
    eleaphProto->registerRPCMethod(strMethod, this, SLOT(packetReceived(EleaphRpcPacket)), true);
}

void EleaphRpcAsyncPacketWaiter::packetReceived(EleaphRpcPacket dataPacket)
{
    this->receivedDataPacket = dataPacket;
    emit packetReady();
}
