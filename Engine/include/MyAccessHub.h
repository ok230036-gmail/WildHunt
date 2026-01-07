#pragma once

#include "MyGameEngine.h"

class MyGameEngine;

class MyAccessHub
{
private:
	static MyGameEngine* m_engine;

public:
	static void SetMyGameEnegine(MyGameEngine* eng)
	{
		m_engine = eng;
	}

	static MyGameEngine* GetMyGameEngine()
	{
		return m_engine;
	}
};
