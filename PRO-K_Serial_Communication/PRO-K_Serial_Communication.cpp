// serial communication PC <--> ReceivedDataString
// T.D. Landzaat (14073595)
// D. Tak
// T. Plevier
// PRO-K Group E
// The Hague university of applied sciences
// Electrical Engineering


// !!This script assumes that the modbus is set to modbusmode!!
#pragma once




#include "stdafx.h"
#include <cstdio>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <atlbase.h>
#include <atlconv.h>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <cmath>
#include <sstream>
#include <bitset>

using namespace std;

int signed_binary_decimal(signed int n) // conversion signed binary to decimal
{
	int decimal = 0, i = 0, remainder;
	while (n != 0)
	{
		remainder = n % 10;
		n /= 10;
		decimal += remainder * 1 << i;
		++i;
	}
	decimal = (decimal + 128) % 256 - 128;
	return decimal;
}

int unsigned_binary_decimal(signed int n) // conversion unsigned binary to decimal
{
	int decimal = 0, i = 0, remainder;
	while (n != 0)
	{
		remainder = n % 10;
		n /= 10;
		decimal += remainder * 1 << i;
		++i;
	}
	return decimal;
}

int signed_hex_to_decimal(std::string signed_hex)
{
	std::string s = signed_hex;
	std::stringstream ss;
	ss << std::hex << s;
	signed n;
	ss >> n;
	std::bitset<16> b(n);
	std::string hex_to_binary = b.to_string();
	int result = 256 * signed_binary_decimal(atoi(hex_to_binary.substr(0, 8).c_str())) + unsigned_binary_decimal(atoi(hex_to_binary.substr(8, 8).c_str()));
	return result;
}

int string_to_unsingen_long(std::string data) {

	const char * c = data.c_str();
	unsigned long data_dec = std::strtoul(c, 0, 16);

	return data_dec;

}

std::wstring s2ws(const std::string& s) //convert string to LPCWSTR
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

void main()
{


	//--------------------Connecting to COM-port--------------------//
	
	HANDLE SerialHandle;
	do {
		std::string ComNumString;
		std::cout << "\nEnter COM port number: ";
		std::cin >> ComNumString;
		std::string ComPrefix = "\\\\.\\COM";

		std::string comID = ComPrefix + ComNumString;

		//covert string to LPCWSTR
		std::wstring stemp = s2ws(comID);
		LPCWSTR ComPort = stemp.c_str();

		SerialHandle = CreateFile(ComPort,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (SerialHandle == INVALID_HANDLE_VALUE)
			printf("Error opening port %s\n", ComPort);
		else
			printf("Port open\n\n", ComPort);

	} while (SerialHandle == INVALID_HANDLE_VALUE);

	//--------------------Setting up connection variables--------------------//

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL Status = GetCommState(SerialHandle, &dcbSerialParams);

	if (Status == FALSE) {
		printf("An error occured\n");
	}
	dcbSerialParams.BaudRate = CBR_57600;
	dcbSerialParams.ByteSize = 7;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = EVENPARITY;

	Status = SetCommState(SerialHandle, &dcbSerialParams);

	if (Status == FALSE)
	{
		printf("Error! in Setting DCB Structure\n\n");
	}
	else
	{
		printf("Baudrate = %d\n", dcbSerialParams.BaudRate);
		printf("ByteSize = %d\n", dcbSerialParams.ByteSize);
		printf("StopBits = %d\n", dcbSerialParams.StopBits);
		printf("Parity   = %d\n\n", dcbSerialParams.Parity);
	}

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(SerialHandle, &timeouts) == FALSE) {
		printf("Error! in Setting Time Outs\n");
	}
	else {
		printf("Setting Serial Port Timeouts Successful\n");
	}
	while (TRUE) {


		//--------------------Writing to COM-port--------------------//

		std::string RequestDataString = ":01040000004BB0";
		LPVOID SendRequestDataString = (LPVOID)RequestDataString.c_str();

		DWORD  dNoOFBytestoWrite;
		DWORD  dNoOfBytesWritten = 0;

		dNoOFBytestoWrite = sizeof(RequestDataString);

		Status = WriteFile(SerialHandle,
			SendRequestDataString,
			dNoOFBytestoWrite,
			&dNoOfBytesWritten,
			NULL);

		if (Status == TRUE)
			printf("Writing succesful");
		else
			printf("\nError %d when writing to data\n", GetLastError());

		//--------------------Receiving Data from COM port--------------------//

		DWORD dwEventMask;
		char  TempChar;
		char  DataBuffer[308];
		DWORD NoBytesRead;
		int i = 0;

		Status = SetCommState(SerialHandle, &dcbSerialParams);

		if (Status == FALSE)
		{
			printf("Error in setting up variables\n");
		}

		Status = SetCommMask(SerialHandle, EV_RXCHAR); //Set windows up to receive data

		if (Status == FALSE)
			printf("Error in setting CommMask\n");

		//--------------------Waiting to receive data--------------------//

		printf("\nWaiting to receive data\n");

		Status = WaitCommEvent(SerialHandle, &dwEventMask, NULL); //Wait for the character to be received

		if (Status == FALSE)
		{
			printf("Error with function WaitCommEvent\n");
		}
		else
		{
			printf("Characters Received");
			do
			{
				Status = ReadFile(SerialHandle, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
				DataBuffer[i] = TempChar;
				i++;
			} while (NoBytesRead > 0);



			//--------------------Writing received data to console--------------------//

			printf("\n\n    ");
			int j = 0;
			for (j = 0; j < i - 1; j++)
				printf("%c", DataBuffer[j]);

		}

		std::string ReceivedDataString = DataBuffer;
		
		std::string v_min = ReceivedDataString.substr(7, 4);
		std::string v_max = ReceivedDataString.substr(11, 4);
		std::string v_avg = ReceivedDataString.substr(15, 4);
		std::string t_min = ReceivedDataString.substr(19, 4);
		std::string t_max = ReceivedDataString.substr(23, 4);
		std::string t_avg = ReceivedDataString.substr(27, 4);
		std::string soc = ReceivedDataString.substr(31, 4);
		std::string i_batt = ReceivedDataString.substr(59, 4);
		std::string i_motor = ReceivedDataString.substr(63, 4);
		std::string motor = ReceivedDataString.substr(71, 4);       //rpm motor
		std::string t_motor = ReceivedDataString.substr(87, 4);
		std::string t_peu = ReceivedDataString.substr(91, 4);
		std::string v_line1 = ReceivedDataString.substr(163, 4);
		std::string v_line2 = ReceivedDataString.substr(167, 4);
		std::string v_line3 = ReceivedDataString.substr(171, 4);
		std::string i_line1 = ReceivedDataString.substr(175, 4);
		std::string i_line2 = ReceivedDataString.substr(179, 4);
		std::string i_line3 = ReceivedDataString.substr(183, 4);

		double t_min_decimal = signed_hex_to_decimal(t_min) / 10.0;
		double t_max_decimal = signed_hex_to_decimal(t_max) / 10.0;
		double t_avg_decimal = signed_hex_to_decimal(t_avg) / 10.0;
		double t_motor_decimal = signed_hex_to_decimal(t_motor) / 10.0;
		double t_peu_decimal = signed_hex_to_decimal(t_peu) / 10.0;

		double i_batt_decimal = signed_hex_to_decimal(i_batt) / 10.0;
		double i_motor_decimal = signed_hex_to_decimal(i_motor) / 10.0;

		double v_min_decimal = string_to_unsingen_long(v_min) / 1000.0;
		double v_max_decimal = string_to_unsingen_long(v_max) / 1000.0;
		double v_avg_decimal = string_to_unsingen_long(v_avg) / 1000.0;

		double soc_decimal = string_to_unsingen_long(soc);
		double motor_decimal = string_to_unsingen_long(motor);

		double v_line1_decimal = string_to_unsingen_long(v_line1) / 10.0;
		double v_line2_decimal = string_to_unsingen_long(v_line2) / 10.0;
		double v_line3_decimal = string_to_unsingen_long(v_line3) / 10.0;

		double i_line1_decimal = string_to_unsingen_long(i_line1) / 100.0;
		double i_line2_decimal = string_to_unsingen_long(i_line2) / 100.0;
		double i_line3_decimal = string_to_unsingen_long(i_line3) / 100.0;

		std::cout << v_min_decimal << " :v_min" << std::endl;
		std::cout << v_max_decimal << " :v_max" << std::endl;
		std::cout << v_avg_decimal << " :v_avg" << std::endl;

		std::cout << motor_decimal << " :motor RPM" << std::endl;
		std::cout << soc_decimal << " :soc" << std::endl;

		std::cout << v_line1_decimal << " :v_line1" << std::endl;
		std::cout << v_line2_decimal << " :v_line2" << std::endl;
		std::cout << v_line3_decimal << " :v_line3" << std::endl;

		std::cout << i_line1_decimal << " :i_line1" << std::endl;
		std::cout << i_line2_decimal << " :i_line2" << std::endl;
		std::cout << i_line3_decimal << " :i_line3" << std::endl;

		std::cout << t_min_decimal << " :t_min" << std::endl;
		std::cout << t_max_decimal << " :t_max" << std::endl;
		std::cout << t_avg_decimal << " :t_avg " << std::endl;
		std::cout << t_peu_decimal << " :t_peu" << std::endl;
		std::cout << t_motor_decimal << " :t_motor" << std::endl;

		std::cout << i_batt_decimal << " :i_batt " << std::endl;
		std::cout << i_motor_decimal << " :i_motor" << std::endl;

		Sleep(15000);

		ofstream myfile;
		myfile.open("Data-Log-Car.txt");
		myfile << v_min_decimal << endl;
		 //timestamp
		myfile.close();


	}
}




