#pragma once

#include "aReader.h"

class ReaderFactory
{
public:
	ReaderFactory();

	virtual ~ReaderFactory();

public:
	static aReader* makeReader(std::string& filePath);
};