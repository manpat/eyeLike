#ifndef PACKET_H
#define PACKET_H

enum class PacketType : byte {
	Ack = 1,
	ServAck,
	SetSmooth,
	GetData,
	Data,

	EnableDebug,

	CantFindFace,
};

#pragma pack(push, 1)
struct Packet {
	PacketType type;
	float data;
};
#pragma pack(pop)

#endif