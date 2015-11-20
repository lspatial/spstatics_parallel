#pragma once

class IDataBlock
{
public:
	virtual ~IDataBlock() {}
	virtual void SerializeTo(void * buffer) {}
	virtual void DeserializeFrom(const void * const buffer, int size) {}
	virtual size_t Size(){return 0;}
	virtual size_t DataSize() {return 0;}
};

