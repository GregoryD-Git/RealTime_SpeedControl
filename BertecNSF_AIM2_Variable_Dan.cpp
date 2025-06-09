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

//first iteration is skipped, therfore set to zero so no belt speed change is missed
int w[76] = {0, 2 ,3 ,2 ,1 ,2 ,1 ,2 ,1 ,2 ,3 ,2 ,0 ,1 ,2 ,0 ,3 ,0 ,1 ,2 ,0 ,1 ,3 ,0 ,2 ,1 ,3 ,1 ,2 ,1 ,2 ,0 ,2 ,3 ,2 ,0 ,1 ,3 ,0 ,2 ,0 ,1 ,0 ,2 ,1 ,0 ,1 ,3 ,2 ,1 ,3 ,2 ,3 ,0 ,3 ,1 ,0 ,3 ,1 ,2 ,1 ,2 ,3 ,2 ,1 ,2 ,1 ,3 ,1 ,3 ,2 ,1 ,3 ,0 ,1 ,2 };

//for calculating time in milliseconds with clock()
//using std::cout;
//using std::endl;

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
float threshold = 200;
//int refreshInterval = 10;      // Refresh period in milliseconds

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

//Initial acceleration
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

	// ----------------------------------------------- CHANGE IP ADDRESS --------------------------------------------------------------------
	// By default assume you want to connect to QTM at the same machine - just for testing
	char pServerAddr[32] = "128.119.66.101"; //"localhost";
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

	std::cout<<"Enter baseline speed: ";
	std::cin>>temp;
	//Set speed dividing by 1.05 to offset the difference between actual and set speed of the treadmill belts
	float L_SPEED = temp/1.05;
	float R_SPEED = temp/1.05;

	std::cout<<"Enter number of ratios: ";
	std::cin>>temp;
	float nr = temp;
	int nratio = nr;

	std::cout<<"Enter limb to perturb (1-Left or 2-Right): ";
	std::cin>>temp;
	float pert_limb = temp;

	std::cout<<"Enter perturbation direction (1-Down or 2-Up): ";
	std::cin>>temp;
	float pert_dir = temp;

	//std::cout<<"Randomize strides between perturbations? (1-YES or 2-NO): ";
	//std::cin>>temp;
	//float pert_ran = temp;
	//float pert_var = 100;

	//if (pert_ran == 2){
	std::cout<<"Enter number of strides between perturbations: ";
	std::cin>>temp;
	float pert_var = temp;
	//}

	std::cout<<"Enter perturbation acc: ";
	std::cin>>temp;
	float acc_pert = temp;

	std::cout<<"Enter decceleration rate: ";
	std::cin>>temp;
	float deccel = temp;

	std::cout<<"Enter number strides to perturb: ";
	std::cin>>temp;
	float pertnum = temp;


	// start treadmill
	//TREADMILL_setSpeed(L_SPEED,R_SPEED,acc);
	//Sleep(1000);

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

	GetForceData();
	fprintf(stream, "%u\t", nFrameNumber);
	fprintf(stream, "%1.4f\t", VForce[0]);
	fprintf(stream, "%1.4f\n", VForce[1]);
	
	fprintf(stream, "Detect Left Heelstrike: ");
	fprintf(stream, "Leg, Event, Frame number\n");

	printf("Detecting HS ...\n");

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
	//Sleep(2000);
	if (Capture == 1){
		poRTProtocol.StartCapture();
		Sleep(1000);
	}

		//Sleep(5000);
		TREADMILL_setSpeed(L_SPEED,R_SPEED,acc);
	

	//initialize random seed
	srand (1);
	int nHS = 0;
	int nVar = 0;

	float rr = 0;
	float r = 0;
	float speed_ratio = 0;

	//Begin perturbation
	while (nHS < pertnum) {
		GetForceData();
		
		if (leftLeg == STANCE){
			if (VForce[0] < threshold){ //change from swing to stance
				leftLeg = SWING;
				
				//Unilateral perturbation
				if (pert_limb == 1 || pert_limb == 3){

					//if (pert_ran == 1){
						//pert_var = rand() %24 + 1;
					//}

					if (nVar == pert_var){			
						r = w[nHS/8];
						//r = rand() % nratio; // random number from 0:nratio inclusive
						//std::cout<< "Ratio index" << std::endl;
						//std::cout  << (float) r << std::endl;

						// NOT USED with specified w vector
						// while loop keeps producing another result until rr != r
						//while (rr == r){
							//r = rand() % nratio;
							//std::cout  << (float) 2 - 0.1*r << std::endl;
						//}
						
						// resets nVar to zero so that the speeds are changed every pert_var times
						nVar = 0; 

						speed_ratio = 2 - (1/(nr-1))*r;
						if (pert_dir == 1){
							TREADMILL_setSpeed(L_SPEED/speed_ratio,R_SPEED,acc_pert);
						}
						else {
							TREADMILL_setSpeed(L_SPEED*speed_ratio,R_SPEED,acc_pert);
						}
						//std::cout<< "Ratio" << std::endl;
						std::cout  <<(float) speed_ratio << std::endl;
						//std::cout  <<(float) nratio  << std::endl;
						rr = r;

						//record event
						poRTProtocol.GetState(eEvent);
							if (eEvent == CRTPacket::EventCaptureStarted) {
								char eventID[12];
								strcpy_s(eventID, "deltaV_L"); //next Target
								poRTProtocol.SetQTMEvent(eventID);
							}
					}
				}
			}
		}
		
		if (leftLeg == SWING){
			if (VForce[0] > threshold){
				leftLeg = STANCE;
					if (pert_limb == 1 || pert_limb == 3){
						nHS++; //count heel strike
						nVar++; //count number of strikes between perts
						Sleep(200);
						std::cout	<< (float) nHS << std::endl;
						std::cout   << (float) nVar << std::endl;
					}
				}
			}
		//}
		if (rightLeg == STANCE){
			if (VForce[1] < threshold){
				rightLeg = SWING;
				
					if (pert_limb == 2 || pert_limb == 3){
						if (nVar == pert_var){			
							//r = rand() % nratio; // random number from 0:nratio inclusive
							//std::cout  << (float) r << std::endl;
							r = w[nHS/8];

							//NOT USED with specified w vector
								// while loop keeps producing another result until rr != r
							//while (rr == r){
								//r = rand() % nratio;
								//std::cout  << (float) r << std::endl;
							//}
						
							// resets nVar to zero so that the speeds are changed every pert_var times
							nVar = 0; 

							speed_ratio = 2 - (1/(nr-1))*r;
							if (pert_dir == 1){
								TREADMILL_setSpeed(L_SPEED,R_SPEED/speed_ratio,acc_pert);
							}
							else {
								TREADMILL_setSpeed(L_SPEED,R_SPEED*speed_ratio,acc_pert);
							}
							std::cout  <<(float) speed_ratio << std::endl;
							rr = r;

							//record event
							poRTProtocol.GetState(eEvent);
								if (eEvent == CRTPacket::EventCaptureStarted) {
									char eventID[12];
									strcpy_s(eventID, "deltaV_R"); //next Target
									poRTProtocol.SetQTMEvent(eventID);
								}
					}
				}
			}
		}
		if (rightLeg == SWING){
			if (VForce[1] > threshold){
					rightLeg = STANCE;

					if (pert_limb == 2 || pert_limb == 3){
						nHS++; //count heel strike
						nVar++; //count number of strikes between perts
						Sleep(200);
						std::cout	<< (float) nHS << std::endl;
						std::cout   << (float) nVar << std::endl;
					}
				}
			}
		} 
}

if (Capture == 1){
			std::cout  << "stop recording" << std::endl;
			poRTProtocol.StopCapture();
			Sleep(500);
	}
	std::cout  << "Complete" << std::endl;
	//Protocol ends and returns speeds to baseline quickly, then stops
	//TREADMILL_setSpeed(L_SPEED/2,R_SPEED/2,acc_pert);
	Sleep(1000);
	TREADMILL_setSpeed(0,0,deccel);
	//Sleep(1000);

};
}
}

	