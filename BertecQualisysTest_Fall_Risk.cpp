// VFall Risk - Gait perturbation protocol for ramped perturbations in either acceleration or decelleration perturbations until a fall occurs
// BertecQualisysTest.cpp : Defines the entry point for the console application.
// Bertec Closed-loop Control

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
FILE *stream;
int nFrameNumber = 0;

//parameters for detecting heel strike
float threshold = 50;
enum Leg
{
	STANCE,
	SWING
};
Leg leftLeg = STANCE;
Leg rightLeg = STANCE;

enum Event
{
	HEELSTRIKE,
	TOEOFF,
	NONE
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
				}
			}
		}
	}
}

Event detectHSTOl(Leg legl) //detect left heelstrike and toeoff
{
	GetForceData();
	if (legl == SWING)
		if (VForce[0] > threshold){ //change from swing to stance
			legl = STANCE;
			return HEELSTRIKE;
		}
		else
			return NONE;
	else if(legl == STANCE)
		if (VForce[0] < threshold) {
			legl = SWING;
			return TOEOFF;
		}
		else
			return NONE;
}

Event detectHSTOr(Leg legr) //detect right heelstrike and toeoff
{
	GetForceData();
	if (legr == SWING)
		if (VForce[1] > threshold){ //change from swing to stance
			legr = STANCE;
			return HEELSTRIKE;
		}
		else
			return NONE;
	else if(legr == STANCE)
		if (VForce[1] < threshold) {
			legr = SWING;
			return TOEOFF;
		}
		else
			return NONE;
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
	char pServerAddr[32] = "128.119.66.72"; //"localhost";
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

	std::cout<<"Unilateral pert == 0, Bilateral == 1: ";
	std::cin>>temp;
	float unibil_pert = temp;

	std::cout<<"HS == 0, SS == 1: ";
	std::cin>>temp;
	float GCP_pert = temp;

	std::cout<<"Enter right perturbation size: ";
	std::cin>>temp;
	float speed_pertr = temp;

	std::cout<<"Enter left perturbation size: ";
	std::cin>>temp;
	float speed_pertl = temp;

	std::cout<<"Enter perturbation acc: ";
	std::cin>>temp;
	float acc_pert = temp;

	std::cout<<"Enter decceleration rate: ";
	std::cin>>temp;
	float deccel = temp;

	std::cout<<"Enter SS perturbation delay: ";
	std::cin>>temp;
	float delay_SS = temp;

	std::cout<<"Enter number strides to pert: ";
	std::cin>>temp;
	float pertnum = temp;


	// start treadmill
	//double acc = 0.5;
	//float L_SPEED = 1.0;
	//float R_SPEED = 1.0;

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
	Sleep(500);
	if (Capture == 1){
		poRTProtocol.StartCapture();
		Sleep(8000);
	}

	//Begin perturbation
	while (nHS < pertnum) {
		GetForceData();
		
		if (leftLeg == STANCE){
			if (VForce[0] < threshold){ //change from swing to stance
				leftLeg = SWING;
				//event capture
				//poRTProtocol.GetState(eEvent);
					//if (eEvent == CRTPacket::EventCaptureStarted) {
						//char eventID[12];
						//strcpy_s(eventID, "LTO"); //next Target
						//poRTProtocol.SetQTMEvent(eventID);
					//}
				//Bilateral perturbation
				if (unibil_pert == 1){
					Sleep(delay_SS);
				TREADMILL_setSpeed(L_SPEED + speed_pertl,R_SPEED + speed_pertr,acc_pert);
				//Sleep(1);
				//TREADMILL_setSpeed(L_SPEED + speed_pertl,R_SPEED + speed_pertr,acc_pert);
				}
				//Unilateral perturbation
				else {
					if (speed_pertl > 0 || GCP_pert == 0){
						//if speed_pertl > 0, return speed from previous perturbation (can be either SS left or HS)
						Sleep(1);
						//return to baseline speed
						TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
						if (GCP_pert == 0){
							//pert left - HS
							TREADMILL_setSpeed(L_SPEED + speed_pertl,R_SPEED,acc_pert); 
						}
					}
				}
				
			}
		}
		
		if (leftLeg == SWING){
			if (VForce[0] > threshold){
				leftLeg = STANCE;
				nHS++; //count heel strike
				//poRTProtocol.GetState(eEvent);
					//if (eEvent == CRTPacket::EventCaptureStarted) {
						//char eventID[12];
						//strcpy_s(eventID, "LIC"); //next Target
						//poRTProtocol.SetQTMEvent(eventID);
					//}
					//SS perturbation only
					if (GCP_pert == 1){
						//pert left - SS
						Sleep(delay_SS);
						if(unibil_pert == 0){
							//for HS pert
							TREADMILL_setSpeed(L_SPEED + speed_pertl,R_SPEED,acc_pert);
						}
						else
						{// for bilateral pert
							TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
						}
					}
			}
		}
		if (rightLeg == STANCE){
			if (VForce[1] < threshold){
				rightLeg = SWING;
				//poRTProtocol.GetState(eEvent);
					//if (eEvent == CRTPacket::EventCaptureStarted) {
						//char eventID[12];
						//strcpy_s(eventID, "RTO"); //next Target
						//poRTProtocol.SetQTMEvent(eventID);
					//}

				if (unibil_pert == 1){
					Sleep(delay_SS);
				TREADMILL_setSpeed(L_SPEED + speed_pertr,R_SPEED + speed_pertl,acc_pert);
				//Sleep(1);
				//TREADMILL_setSpeed(L_SPEED + speed_pertr,R_SPEED + speed_pertl,acc_pert);
				}
				
				else{
					if (speed_pertr > 0 || GCP_pert == 0){
						Sleep(1);
						TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
						if (GCP_pert == 0){
							TREADMILL_setSpeed(L_SPEED,R_SPEED + speed_pertr,acc_pert); 
						}
					}
				}
			}
		}
		if (rightLeg == SWING){
			if (VForce[1] > threshold){
					rightLeg = STANCE;
					//poRTProtocol.GetState(eEvent);
						//if (eEvent == CRTPacket::EventCaptureStarted) {
							//char eventID[12];
							//strcpy_s(eventID, "RIC"); //next Target
							//poRTProtocol.SetQTMEvent(eventID);
						//}

					if (GCP_pert == 1){
						Sleep(delay_SS);
						if (unibil_pert == 0){
							TREADMILL_setSpeed(L_SPEED,R_SPEED + speed_pertr,acc_pert);
						}
						else
						{
							TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);

						}
					}
				}
		}
	} 
	//Protocol ends and returns speeds to baseline quickly, then stops
	TREADMILL_setSpeed(L_SPEED,R_SPEED,acc_pert);
	//Sleep(500);
	//TREADMILL_setSpeed(0,0,deccel);
	//Sleep(1000);

};
}
}
}
	