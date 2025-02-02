// serial communication PC <--> ReceivedDataString
// T.D. Landzaat (14073595)
// D. Tak (15074056)
// T. Plevier (15132277)
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
#include <ctime>

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
		string ComNumString;
		cout << "\nEnter COM port number: ";
		cin >> ComNumString;
		string ComPrefix = "\\\\.\\COM";

		std::string comID = ComPrefix + ComNumString;

		//covert string to LPCWSTR
		std::wstring stemp = s2ws(comID);
		LPCWSTR ComPort = stemp.c_str();

		SerialHandle =	CreateFile(
							ComPort,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							0,
							NULL);

		if (SerialHandle == INVALID_HANDLE_VALUE)
			cout << "Error opening port: " << ComPort << endl;
		else
			cout<< "Connected to COM port: "<< ComPort << endl;

	} while (SerialHandle == INVALID_HANDLE_VALUE);

	//--------------------Setting up connection variables--------------------//

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL Status = GetCommState(SerialHandle, &dcbSerialParams);

	if (Status == FALSE) {
		cout << "Error in function GetCommState" << endl;
	}
	dcbSerialParams.BaudRate = CBR_57600;
	dcbSerialParams.ByteSize = 7;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = EVENPARITY;

	Status = SetCommState(SerialHandle, &dcbSerialParams);

	if (Status == FALSE)
	{
		cout << "Error in function SetCommState" << endl;
	}
	else
	{
		cout << "Baudrate = " << dcbSerialParams.BaudRate << endl;
		cout << "ByteSize = " << dcbSerialParams.ByteSize << endl;
		cout << "StopBits = " << dcbSerialParams.StopBits << endl;
		cout << "Parity = " << dcbSerialParams.Parity << endl;
	}

	COMMTIMEOUTS timeouts = { 0 };

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (SetCommTimeouts(SerialHandle, &timeouts) == FALSE) {
		cout << "Error in setting up time-outs" << endl;
	}
	else {
		cout << "Setting Serial Port Timeouts Successful" << endl;
	}

	ofstream DataLog;
	DataLog.open("Data-Log-Car.txt");

	while (TRUE) {

		//--------------------Write data to COM-port--------------------//

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
			cout << "Writing succesful" << endl;
		else
			cout << "Error when writing data: " << GetLastError() << endl;

		//--------------------Receiving Data from COM port--------------------//

		DWORD dwEventMask;
		char  TempChar;
		char  DataBuffer[308];
		DWORD NoBytesRead;
		int i = 0;

		Status = SetCommState(SerialHandle, &dcbSerialParams);

		if (Status == FALSE)
		{
			cout << "Error in setting up variables" << endl;
		}

		Status = SetCommMask(SerialHandle, EV_RXCHAR); //Set windows up to receive data

		if (Status == FALSE)
			cout << "Error in setting CommMask" << endl;

		//--------------------Waiting to receive data--------------------//

		cout << "Waiting to receive data" << endl;

		Status = WaitCommEvent(SerialHandle, &dwEventMask, NULL); //Wait for the character to be received

		if (Status == FALSE)
		{
			cout << "Error with function WaitCommEvent" << endl;
		}
		else
		{
			cout << "Receiving data..." << endl;
			do
			{
				Status = ReadFile(SerialHandle, &TempChar, sizeof(TempChar), &NoBytesRead, NULL);
				DataBuffer[i] = TempChar;
				i++;
			} while (NoBytesRead > 0);



			//--------------------Writing received data to console--------------------//

			cout << endl;
			int j = 0;
			for (j = 0; j < i - 1; j++)
				cout << DataBuffer[j];

		}
		
		//--------------------Convert received data to correct datastructure--------------------//

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

		//--------------------Show data on console--------------------//

		cout << v_min_decimal << " :v_min" << endl;
		cout << v_max_decimal << " :v_max" << endl;
		cout << v_avg_decimal << " :v_avg" << endl;

		cout << motor_decimal << " :motor RPM" << endl;
		cout << soc_decimal << " :soc" << endl;

		cout << v_line1_decimal << " :v_line1" << endl;
		cout << v_line2_decimal << " :v_line2" << endl;
		cout << v_line3_decimal << " :v_line3" << endl;

		cout << i_line1_decimal << " :i_line1" << endl;
		cout << i_line2_decimal << " :i_line2" << endl;
		cout << i_line3_decimal << " :i_line3" << endl;

		cout << t_min_decimal << " :t_min" << endl;
		cout << t_max_decimal << " :t_max" << endl;
		cout << t_avg_decimal << " :t_avg " << endl;
		cout << t_peu_decimal << " :t_peu" << endl;
		cout << t_motor_decimal << " :t_motor" << endl;

		cout << i_batt_decimal << " :i_batt " << endl;
		cout << i_motor_decimal << " :i_motor" << endl;

		//--------------------Writing read data to .txt file--------------------//

		time_t time_now = time(0);
		char* time_now_readable = ctime(&time_now);

		DataLog << "&&-------------------------------------------------------&&"  << endl;
		DataLog << time_now_readable;
		DataLog << "&&-------------------------------------------------------&&" << endl << endl;

		DataLog << v_min_decimal << " :v_min" << endl;
		DataLog << v_max_decimal << " :v_max" << endl;
		DataLog << v_avg_decimal << " :v_avg" << endl;

		DataLog << motor_decimal << " :motor RPM" << endl;
		DataLog << soc_decimal << " :soc" << endl;

		DataLog << v_line1_decimal << " :v_line1" << endl;
		DataLog << v_line2_decimal << " :v_line2" << endl;
		DataLog << v_line3_decimal << " :v_line3" << endl;

		DataLog << i_line1_decimal << " :i_line1" << endl;
		DataLog << i_line2_decimal << " :i_line2" << endl;
		DataLog << i_line3_decimal << " :i_line3" << endl;

		DataLog << t_min_decimal << " :t_min" << endl;
		DataLog << t_max_decimal << " :t_max" << endl;
		DataLog << t_avg_decimal << " :t_avg " << endl;
		DataLog << t_peu_decimal << " :t_peu" << endl;
		DataLog << t_motor_decimal << " :t_motor" << endl;

		DataLog << i_batt_decimal << " :i_batt " << endl;
		DataLog << i_motor_decimal << " :i_motor" << endl << endl;

		Sleep(15000);
	}
	
}




