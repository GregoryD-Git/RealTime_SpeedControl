// BertecQualisysTest.cpp : Defines the entry point for the console application.
// Bertec Closed-loop Control
// Sym(v2.1) uses the A/P GRF to change belt speeds suring mid-stance

#include "stdafx.h"

#include "RTProtocol.h"
#define QTM_RT_SERVER_BASE_PORT 22222

#include <windows.h>  // for MS Windows
#include <float.h>
#include <stdio.h>
#include <conio.h>
#include <string>
#include <iostream>

#include <cstdlib> 

#include "stdafx.h"

#include <treadmill-remote.h>

#include <condition_variable>

#include <vector>
#include <time.h>

std::condition_variable cv;

//#include <vector>
//#include <algorithm>
//#include <functional>

#define BUFSIZE 256

//QTM RT Protocol
CRTProtocol				poRTProtocol;
CRTPacket*				pRTPacket;
CRTPacket::EPacketType	eType;
CRTPacket::EEvent       eEvent;	

float VForce[2];
float AForce[2];
FILE *stream;
int nFrameNumber = 0;

//parameters for detecting heel strike
float thresholdV = 100;
float thresholdA = 70;
enum Leg
{
	STANCE,
	SWING
};
Leg leftLeg = STANCE;
Leg rightLeg = STANCE;

enum LegA
{
	NEG,
	POS
};
LegA leftLegA = NEG;
LegA rightLegA = NEG;

enum Event
{
	HEELSTRIKE,
	TOEOFF,
	NONE
};

enum EventA
{
	EARLY,
	LATE,
	NOTST
};

void GetForceData() {
	bool bDataAvailable;
	poRTProtocol.ReadForceSettings(bDataAvailable);
	unsigned int nCount = poRTProtocol.GetForcePlateCount();
	if (nCount > 0)
	{
		CRTPacket::SForce sForce;
		poRTProtocol.GetCurrentFrame(CRTProtocol::cComponentForceSingle);
		if ((poRTProtocol.ReceiveRTPacket(eType, true)) && (CRTPacket::PacketData))
		{
			pRTPacket = poRTProtocol.GetRTPacket();
			nFrameNumber = pRTPacket->GetFrameNumber();
			for (unsigned int iPlate = 0; iPlate < nCount; iPlate++)
			{
				if (pRTPacket->GetForceSingleData(iPlate, sForce))
				{
					VForce[iPlate] = sForce.fForceZ;
					AForce[iPlate] = sForce.fForceY;
				}
			}
		}
	}
}

// Left heel strike and toe off
Event detectHSTOl(Leg legl) //detect left heelstrike and toeoff
{
	GetForceData();
	if (legl == SWING)
		if (VForce[0] > thresholdV){ //change from swing to stance
			legl = STANCE;
			return HEELSTRIKE;

		}
		else
			return NONE;
	else if(legl == STANCE)
		if (VForce[0] < thresholdV) {
			legl = SWING;
			return TOEOFF;
		}
		else
			return NONE;
}
// Left mid-stance
EventA detectMidSTl(LegA leglA) // detect left mid-stance
{
	GetForceData();
	if (leglA == NEG)
		if (AForce[0] < thresholdA){ //change from neg to pos stance
			leglA = POS;
			return LATE;
		}
		else
			return NOTST;
	else if(leglA == POS)
		if (AForce[0] > thresholdA) {
			leglA = NEG;
			return EARLY;
		}
		else
			return NOTST;
}

// Right heel strike and toe off
Event detectHSTOr(Leg legr) //detect right heelstrike and toeoff
{
	GetForceData();
	if (legr == SWING)
		if (VForce[1] > thresholdV){ //change from swing to stance
			legr = STANCE;
			return HEELSTRIKE;
		}
		else
			return NONE;
	else if(legr == STANCE)
		if (VForce[1] < thresholdV) {
			legr = SWING;
			return TOEOFF;
		}
		else
			return NONE;
}

// Right mid-stance
EventA detectMidSTr(LegA legrA) // detect left mid-stance
{
	GetForceData();
	if (legrA == NEG)
		if (AForce[1] < thresholdA){ //change from neg to pos stance
			legrA = POS;
			return LATE;
		}
		else
			return NOTST;
	else if(legrA == POS)
		if (AForce[1] > thresholdA) {
			legrA = NEG;
			return EARLY;
		}
		else
			return NOTST;
}

double acc = 0.5;

int _tmain(int argc, _TCHAR* argv[])
{
	//Engine *ep;

	//char buffer[BUFSIZE+1];

	if(!TREADMILL_initialize("127.0.0.1","4000")==0)
	{
		std::cout<<"couldn't connect to treadmill using default connection settings";
		Sleep(2000);
		exit(1);
	}
	else
	{
		std::cout<<"connected to treadmill...\n";
		Sleep(1000);
	}


	// By default assume you want to connect to QTM at the same machine - just for testing
	char pServerAddr[32] = "128.119.66.89"; //"localhost";
	int	nMajorVersion = 1;
	int nMinorVersion = 10;
	if (!poRTProtocol.Connect(pServerAddr, QTM_RT_SERVER_BASE_PORT, 0, nMajorVersion, nMinorVersion)) {
		std::cout<<"not connected to Qualysis ...\n";
		Sleep(2000);
	}

	printf("Enter File Name : ");
	char pStr[256];
	gets_s(pStr, sizeof(pStr));
	stream = fopen(pStr, "a+");	
	fprintf(stream, "BertecQualisysTest: ===================\n");


	float temp = 0;

	std::cout<<"Capture (1 == yes): ";
	std::cin>>temp;
	float Capture = temp;

	std::cout<<"Enter speed base: ";
	std::cin>>temp;
	float L_SPEED = temp;
	float R_SPEED = temp;

	std::cout<<"Enter perturbation size: ";
	std::cin>>temp;
	float speed_pert = temp;

	std::cout<<"Enter perturbation acc: ";
	std::cin>>temp;
	float acc_pert = temp;

	std::cout<<"Enter decceleration rate: ";
	std::cin>>temp;
	float deccel = temp;

	//std::cout<<"Enter fast delay: ";
	//std::cin>>temp;
	//float delay_fast = temp;

	//std::cout<<"Enter slow delay: ";
	//std::cin>>temp;
	//float delay_slow = temp;

	std::cout<<"Enter perturbation limb (0 == left slow, 1 == right, 2 == both): ";
	std::cin>>temp;
	float pert_limb = temp;

	std::cout<<"Enter number of strides: ";
	std::cin>>temp;
	float pertnum = temp;

	std::cout<<"Enter start delay (seconds): ";
	std::cin>>temp;
	float start_delay = temp;

	TREADMILL_setSpeed(L_SPEED,R_SPEED,acc);
	Sleep(1000);
	

	// Get settings from QTM		
	if (poRTProtocol.Connect(pServerAddr, QTM_RT_SERVER_BASE_PORT, 0, nMajorVersion, nMinorVersion)) {
		poRTProtocol.TakeControl();
		poRTProtocol.NewMeasurement();
		poRTProtocol.GetState(eEvent);
		while (eEvent != CRTPacket::EventConnected)
			poRTProtocol.GetState(eEvent);
		//poRTProtocol.StartCapture();
		std::cout<<"connected to QTM ...\n";
		Sleep(1000);
	}

	//poRTProtocol.StartCapture();

	GetForceData();
	fprintf(stream, "%u\t", nFrameNumber);
	fprintf(stream, "%1.4f\t", VForce[0]);
	fprintf(stream, "%1.4f\n", VForce[1]);
	
	fprintf(stream, "Detect Left Heelstrike: ");
	fprintf(stream, "Leg, Event, Frame number\n");

	printf("Detecting HS ...\n");
	int nHS = 0;

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

while(keepListening)
{
ReadConsoleInput(hIn, &InRec, 1, &NumRead);
bKeyPressDown = InRec.Event.KeyEvent.bKeyDown;

if(InRec.EventType == KEY_EVENT && bKeyPressDown)
{
if(InRec.Event.KeyEvent.wVirtualKeyCode == VK_OEM_PLUS)
{

	// Wait period to begin recording
	Sleep(start_delay*1000);
	if (Capture == 1){
		poRTProtocol.StartCapture();
		std::cout  << "starting capture" << std::endl;
		Sleep(1000);
	}

	//Begin perturbation
	while (nHS < pertnum) {
		GetForceData();
		
		if (leftLeg == STANCE){
			if (VForce[0] < thresholdV){ //change from swing to stance
				leftLeg = SWING;
				leftLegA = NEG;
			}
		}
		
		if (leftLeg == SWING){
			if (VForce[0] > thresholdV){
				leftLeg = STANCE;
				nHS++; //count heel strike
				
				if (leftLegA == NEG){
					if (AForce[0] < thresholdA){
						leftLegA = POS;
						if (pert_limb == 1){
							//Sleep(delay_fast);
							TREADMILL_setSpeed(L_SPEED + speed_pert,R_SPEED + speed_pert,acc_pert);
						}
						else if (pert_limb == 0){
							//Sleep(delay_slow);
							TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
						}
						else if (pert_limb == 2){
							//Sleep(delay_slow);
							TREADMILL_setSpeed(L_SPEED,R_SPEED + speed_pert,acc_pert);
						}
					}
				}
			}
		}
		if (rightLeg == STANCE){
			if (VForce[1] < thresholdV){
				rightLeg = SWING;
				rightLegA = NEG;
			}
		}
		if (rightLeg == SWING){
			if (VForce[1] > thresholdV){
					rightLeg = STANCE;
					//poRTProtocol.GetState(eEvent);
						//if (eEvent == CRTPacket::EventCaptureStarted) {
							//char eventID[12];
							//strcpy_s(eventID, "RIC"); //next Target
							//poRTProtocol.SetQTMEvent(eventID);
						//}

					if (rightLegA == NEG){
						if (AForce[1] < thresholdA){
						rightLegA = POS;
							if (pert_limb == 0){
								//Sleep(delay_fast);
								TREADMILL_setSpeed(L_SPEED + speed_pert,R_SPEED + speed_pert,acc_pert);
							}
							else if (pert_limb == 1){
								//Sleep(delay_slow);
								TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
							}
							else if (pert_limb == 2){
								//Sleep(delay_slow);
								TREADMILL_setSpeed(L_SPEED + speed_pert,R_SPEED,acc_pert);
							}
						}
					}
				}
		}
	} 

	if (Capture == 1){
			std::cout  << "stop recording" << std::endl;
			poRTProtocol.StopCapture();
			Sleep(1500);
	}
	//Protocol ends and returns speeds to baseline quickly, then stops
	//TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
	//Sleep(1000);
	TREADMILL_setSpeed(0,0,deccel);
	//Sleep(1000);

}; // keyboard key event initialize?
} // keyboard key event start
} // start while-loop for listening to keyboard events
}
	