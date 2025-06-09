// BertecRemote.cpp : Defines the entry point for the console application.
// randomly changes a both belts' speed (within a desired speed range) after a certain duration

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <treadmill-remote.h>
#include <engine.h> 
#include <vector>
#include <time.h>
#include <conio.h>
#include <cstdlib>
#include <iomanip>
#define BUFSIZE 256


double acc = 1.0;
float newSpeed;

int _tmain(int argc, _TCHAR* argv[])
{
	//Engine *ep;
	
	char buffer[BUFSIZE+1];

	if(!TREADMILL_initialize("127.0.0.1","4000")==0)
	{
		std::cout<<"Couldn't connect to treadmill using default connection settings";
		exit(1);
	}

	float temp = 0;
	std::cout<<"Enter speed (m/s): ";
	std::cin>>temp;
	float L_SPEED = temp/1.05;
	//std::cout<<"Enter start speed (right): ";
	//std::cin>>temp;
	float R_SPEED = temp/1.05;

	std::cout<<"Enter perturbation percentage (0-100% of initial speed): ";
	std::cin>>temp;
	float pert_perc = temp/100;

	std::cout<<"Enter duration between speed-changes (s): ";
	std::cin>>temp;
	float time_pert = temp;

	TREADMILL_setSpeed(L_SPEED, R_SPEED, acc);

	//Listen to keyboard control
	HANDLE hIn;
	HANDLE hOut;
	COORD KeyWhere;
	COORD EndWhere;

	int KeyEvents = 0;
	INPUT_RECORD InRec;
	DWORD NumRead;
	int bKeyPressDown = 0;

	BOOL keepListening = true;

	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_INPUT_HANDLE);

	//int count = 1;
	std::cout << std::setprecision(3); // set precision to 3 so 2-3 decimals are shown when new speed is printed in command prompt

	while(keepListening)
	{
		Sleep(time_pert*1000);
		float r = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX/2))-1; // r is random number between -1 and 1
		newSpeed = L_SPEED + L_SPEED * r * pert_perc; // sets new speed as a random percentage within the desired range
		std::cout	<< (float) newSpeed << std::endl; // prints new speed in command prompt
		TREADMILL_setSpeed(newSpeed,newSpeed,acc);
		/*ReadConsoleInput(hIn, &InRec, 1, &NumRead);
		bKeyPressDown = InRec.Event.KeyEvent.bKeyDown;

		if(InRec.EventType == KEY_EVENT && bKeyPressDown)
		{
			if(InRec.Event.KeyEvent.wVirtualKeyCode == VK_OEM_PLUS)
			{
			}
			if(InRec.Event.KeyEvent.wVirtualKeyCode == VK_OEM_MINUS)
			{
			}
			if(InRec.Event.KeyEvent.wVirtualKeyCode == VK_UP)	
			{
			}
			if(InRec.Event.KeyEvent.wVirtualKeyCode == VK_DOWN)
			{
			}
		}*/
	}
}

