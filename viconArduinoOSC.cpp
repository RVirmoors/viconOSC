// viconOSC.cpp : Defines the entry point for the console application.
//

#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"
#define OUTPUT_BUFFER_SIZE 1024

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) OMG Plc 2009.
// All rights reserved.  This software is protected by copyright
// law and international treaties.  No part of this software / document
// may be reproduced or distributed in any form or by any means,
// whether transiently or incidentally to some other use of this software,
// without the written permission of the copyright owner.
//
///////////////////////////////////////////////////////////////////////////////

#include "Client.h"

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <cassert>
#include <ctime>
#include <vector>
#include <string.h>

#ifdef WIN32
#include <conio.h>   // For _kbhit()
#include <cstdio>   // For getchar()
#include <windows.h> // For Sleep()
#else
#include <unistd.h> // For sleep()
#endif // WIN32

#include <time.h>

using namespace ViconDataStreamSDK::CPP;

#define output_stream if(!LogFile.empty()) ; else std::cout 

namespace
{
	std::string Adapt(const bool i_Value)
	{
		return i_Value ? "True" : "False";
	}

	std::string Adapt(const Direction::Enum i_Direction)
	{
		switch (i_Direction)
		{
		case Direction::Forward:
			return "Forward";
		case Direction::Backward:
			return "Backward";
		case Direction::Left:
			return "Left";
		case Direction::Right:
			return "Right";
		case Direction::Up:
			return "Up";
		case Direction::Down:
			return "Down";
		default:
			return "Unknown";
		}
	}

	std::string Adapt(const DeviceType::Enum i_DeviceType)
	{
		switch (i_DeviceType)
		{
		case DeviceType::ForcePlate:
			return "ForcePlate";
		case DeviceType::Unknown:
		default:
			return "Unknown";
		}
	}

	std::string Adapt(const Unit::Enum i_Unit)
	{
		switch (i_Unit)
		{
		case Unit::Meter:
			return "Meter";
		case Unit::Volt:
			return "Volt";
		case Unit::NewtonMeter:
			return "NewtonMeter";
		case Unit::Newton:
			return "Newton";
		case Unit::Kilogram:
			return "Kilogram";
		case Unit::Second:
			return "Second";
		case Unit::Ampere:
			return "Ampere";
		case Unit::Kelvin:
			return "Kelvin";
		case Unit::Mole:
			return "Mole";
		case Unit::Candela:
			return "Candela";
		case Unit::Radian:
			return "Radian";
		case Unit::Steradian:
			return "Steradian";
		case Unit::MeterSquared:
			return "MeterSquared";
		case Unit::MeterCubed:
			return "MeterCubed";
		case Unit::MeterPerSecond:
			return "MeterPerSecond";
		case Unit::MeterPerSecondSquared:
			return "MeterPerSecondSquared";
		case Unit::RadianPerSecond:
			return "RadianPerSecond";
		case Unit::RadianPerSecondSquared:
			return "RadianPerSecondSquared";
		case Unit::Hertz:
			return "Hertz";
		case Unit::Joule:
			return "Joule";
		case Unit::Watt:
			return "Watt";
		case Unit::Pascal:
			return "Pascal";
		case Unit::Lumen:
			return "Lumen";
		case Unit::Lux:
			return "Lux";
		case Unit::Coulomb:
			return "Coulomb";
		case Unit::Ohm:
			return "Ohm";
		case Unit::Farad:
			return "Farad";
		case Unit::Weber:
			return "Weber";
		case Unit::Tesla:
			return "Tesla";
		case Unit::Henry:
			return "Henry";
		case Unit::Siemens:
			return "Siemens";
		case Unit::Becquerel:
			return "Becquerel";
		case Unit::Gray:
			return "Gray";
		case Unit::Sievert:
			return "Sievert";
		case Unit::Katal:
			return "Katal";

		case Unit::Unknown:
		default:
			return "Unknown";
		}
	}
#ifdef WIN32
	bool Hit()
	{
		bool hit = false;
		while (_kbhit())
		{
			getchar();
			hit = true;
		}
		return hit;
	}
#endif
}

int main(int argc, char* argv[])
{
	// define OSC send target
	UdpTransmitSocket transmitSocket(IpEndpointName("localhost", 444));

	// Program options

	std::string HostName = "localhost:801";
	if (argc > 1)
	{
		HostName = argv[1];
	}

	// log contains:
	// version number
	// log of framerate over time
	// --multicast
	// kill off internal app
	std::string LogFile = "";
	std::string MulticastAddress = "244.0.0.0:44801";
	bool ConnectToMultiCast = false;
	bool EnableMultiCast = false;
	bool EnableHapticTest = false;
	bool bReadCentroids = false;
	std::vector<std::string> HapticOnList(0);
	for (int a = 2; a < argc; ++a)
	{
		std::string arg = argv[a];
		if (arg == "--help")
		{
			std::cout << argv[0] << " <HostName>: allowed options include:\n  --log_file <LogFile> --enable_multicast <MulticastAddress:Port> --connect_to_multicast <MulticastAddress:Port> --help --enable_haptic_test <DeviceName> --centroids" << std::endl;
			return 0;
		}
		else if (arg == "--log_file")
		{
			if (a < argc)
			{
				LogFile = argv[a + 1];
				std::cout << "Using log file <" << LogFile << "> ..." << std::endl;
				++a;
			}
		}
		else if (arg == "--enable_multicast")
		{
			EnableMultiCast = true;
			if (a < argc)
			{
				MulticastAddress = argv[a + 1];
				std::cout << "Enabling multicast address <" << MulticastAddress << "> ..." << std::endl;
				++a;
			}
		}
		else if (arg == "--connect_to_multicast")
		{
			ConnectToMultiCast = true;
			if (a < argc)
			{
				MulticastAddress = argv[a + 1];
				std::cout << "connecting to multicast address <" << MulticastAddress << "> ..." << std::endl;
				++a;
			}
		}
		else if (arg == "--enable_haptic_test")
		{
			EnableHapticTest = true;
			++a;
			if (a < argc)
			{
				//assuming no haptic device name starts with "--"
				while (a < argc && strncmp(argv[a], "--", 2) != 0)
				{
					HapticOnList.push_back(argv[a]);
					++a;
				}
			}
		}
		else if (arg == "--centroids")
		{
			bReadCentroids = true;
		}
		else
		{
			std::cout << "Failed to understand argument <" << argv[a] << ">...exiting" << std::endl;
			return 1;
		}
	}

	std::ofstream ofs;
	if (!LogFile.empty())
	{
		ofs.open(LogFile.c_str());
		if (!ofs.is_open())
		{
			std::cout << "Could not open log file <" << LogFile << ">...exiting" << std::endl;
			return 1;
		}
	}
	// Make a new client
	Client MyClient;

	for (int i = 0; i != 3; ++i) // repeat to check disconnecting doesn't wreck next connect
	{
		// Connect to a server
		std::cout << "Connecting to " << HostName << " ..." << std::flush;
		while (!MyClient.IsConnected().Connected)
		{
			// Direct connection

			bool ok = false;
			if (ConnectToMultiCast)
			{
				// Multicast connection
				ok = (MyClient.ConnectToMulticast(HostName, MulticastAddress).Result == Result::Success);

			}
			else
			{
				ok = (MyClient.Connect(HostName).Result == Result::Success);
			}
			if (!ok)
			{
				std::cout << "Warning - connect failed..." << std::endl;
			}


			std::cout << ".";
#ifdef WIN32
			Sleep(1000);
#else
			Sleep(1);
#endif
		}
		std::cout << std::endl;

		// Enable some different data types
		//		MyClient.EnableSegmentData();
		MyClient.EnableMarkerData();
		//		MyClient.EnableUnlabeledMarkerData();
		//		MyClient.EnableDeviceData();
		/*	if (bReadCentroids)
		{
		MyClient.EnableCentroidData();
		}*/

		//		std::cout << "Segment Data Enabled: " << Adapt(MyClient.IsSegmentDataEnabled().Enabled) << std::endl;
		std::cout << "Marker Data Enabled: " << Adapt(MyClient.IsMarkerDataEnabled().Enabled) << std::endl;
		//		std::cout << "Unlabeled Marker Data Enabled: " << Adapt(MyClient.IsUnlabeledMarkerDataEnabled().Enabled) << std::endl;
		//		std::cout << "Device Data Enabled: " << Adapt(MyClient.IsDeviceDataEnabled().Enabled) << std::endl;
		//		std::cout << "Centroid Data Enabled: " << Adapt(MyClient.IsCentroidDataEnabled().Enabled) << std::endl;

		// Set the streaming mode
		//MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPull );
		// MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPullPreFetch );
		MyClient.SetStreamMode(ViconDataStreamSDK::CPP::StreamMode::ServerPush);

		// Set the global up axis
		MyClient.SetAxisMapping(Direction::Forward,
			Direction::Left,
			Direction::Up); // Z-up
							// MyClient.SetGlobalUpAxis( Direction::Forward, 
							//                           Direction::Up, 
							//                           Direction::Right ); // Y-up

		Output_GetAxisMapping _Output_GetAxisMapping = MyClient.GetAxisMapping();
		std::cout << "Axis Mapping: X-" << Adapt(_Output_GetAxisMapping.XAxis)
			<< " Y-" << Adapt(_Output_GetAxisMapping.YAxis)
			<< " Z-" << Adapt(_Output_GetAxisMapping.ZAxis) << std::endl;

		// Discover the version number
		Output_GetVersion _Output_GetVersion = MyClient.GetVersion();
		std::cout << "Version: " << _Output_GetVersion.Major << "."
			<< _Output_GetVersion.Minor << "."
			<< _Output_GetVersion.Point << std::endl;

		if (EnableMultiCast)
		{
			assert(MyClient.IsConnected().Connected);
			MyClient.StartTransmittingMulticast(HostName, MulticastAddress);
		}

		size_t FrameRateWindow = 1000; // frames
		size_t Counter = 0;
		clock_t LastTime = clock();
		// Loop until a key is pressed
#ifdef WIN32
		while (!Hit())
#else
		while (true)
#endif
		{
			// Get a frame
			//			output_stream << "Waiting for new frame...";
			while (MyClient.GetFrame().Result != Result::Success)
			{
				// Sleep a little so that we don't lumber the CPU with a busy poll
#ifdef WIN32
				Sleep(200);
#else
				sleep(1);
#endif

				output_stream << ".";
			}
			//			output_stream << std::endl;
			/*if (++Counter == FrameRateWindow)
			{
			clock_t Now = clock();
			double FrameRate = (double)(FrameRateWindow * CLOCKS_PER_SEC) / (double)(Now - LastTime);
			if (!LogFile.empty())
			{
			time_t rawtime;
			struct tm * timeinfo = NULL;
			time(&rawtime);
			//timeinfo = localtime(&rawtime);
			#ifdef WIN32
			localtime_s(timeinfo, &rawtime);
			#else
			//localtime_r(&rawtime, timeinfo);
			#endif

			#ifdef WIN32
			char str[26];
			ofs << "Frame rate = " << FrameRate << " at " << asctime_s(str, 26, timeinfo) << std::endl;
			#else
			ofs << "Frame rate = " << FrameRate << " at " << asctime(timeinfo) << std::endl;
			#endif
			}

			LastTime = Now;
			Counter = 0;
			}*/

			// Get the frame number
			//		Output_GetFrameNumber _Output_GetFrameNumber = MyClient.GetFrameNumber();
			//		output_stream << "Frame Number: " << _Output_GetFrameNumber.FrameNumber << std::endl;

			/*	char buffer[OUTPUT_BUFFER_SIZE];
			osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

			p << osc::BeginBundleImmediate
			<< osc::BeginMessage("FrameNumber")
			<< true << (osc::int32)_Output_GetFrameNumber.FrameNumber << osc::EndMessage
			<< osc::EndBundle;

			transmitSocket.Send(p.Data(), p.Size());
			*/
			/*
			if (EnableHapticTest == true)
			{
			for (size_t i = 0; i < HapticOnList.size(); ++i)
			{
			if (Counter % 2 == 0)
			{
			Output_SetApexDeviceFeedback Output = MyClient.SetApexDeviceFeedback(HapticOnList[i], true);
			if (Output.Result == Result::Success)
			{
			output_stream << "Turn haptic feedback on for device: " << HapticOnList[i] << std::endl;
			}
			else if (Output.Result == Result::InvalidDeviceName)
			{
			output_stream << "Device doesn't exist: " << HapticOnList[i] << std::endl;
			}
			}
			if (Counter % 20 == 0)
			{
			Output_SetApexDeviceFeedback Output = MyClient.SetApexDeviceFeedback(HapticOnList[i], false);

			if (Output.Result == Result::Success)
			{
			output_stream << "Turn haptic feedback off for device: " << HapticOnList[i] << std::endl;
			}
			}
			}
			}*/

			//Output_GetFrameRate Rate = MyClient.GetFrameRate();
			//std::cout << "Frame rate: " << Rate.FrameRateHz << std::endl;
			/*
			// Get the timecode
			Output_GetTimecode _Output_GetTimecode = MyClient.GetTimecode();

			output_stream << "Timecode: "
			<< _Output_GetTimecode.Hours << "h "
			<< _Output_GetTimecode.Minutes << "m "
			<< _Output_GetTimecode.Seconds << "s "
			<< _Output_GetTimecode.Frames << "f "
			<< _Output_GetTimecode.SubFrame << "sf "
			<< Adapt(_Output_GetTimecode.FieldFlag) << " "
			<< _Output_GetTimecode.Standard << " "
			<< _Output_GetTimecode.SubFramesPerFrame << " "
			<< _Output_GetTimecode.UserBits << std::endl << std::endl;

			*/

			/*		char buffer[OUTPUT_BUFFER_SIZE];
			osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

			p << osc::BeginBundleImmediate
			<< osc::BeginMessage("TimeCode")
			<< true << (osc::int32)_Output_GetTimecode.Seconds << (osc::int32)_Output_GetTimecode.Frames << osc::EndMessage
			<< osc::EndBundle;

			transmitSocket.Send(p.Data(), p.Size());
			*/

			// Get the latency
			//		output_stream << "Latency: " << MyClient.GetLatencyTotal().Total << "s" << std::endl;
			/*
			for (unsigned int LatencySampleIndex = 0; LatencySampleIndex < MyClient.GetLatencySampleCount().Count; ++LatencySampleIndex)
			{
			std::string SampleName = MyClient.GetLatencySampleName(LatencySampleIndex).Name;
			double      SampleValue = MyClient.GetLatencySampleValue(SampleName).Value;

			output_stream << "  " << SampleName << " " << SampleValue << "s" << std::endl;
			}
			output_stream << std::endl;
			*/
			// Count the number of subjects
			unsigned int SubjectCount = MyClient.GetSubjectCount().SubjectCount;
			//	output_stream << "Subjects (" << SubjectCount << "):" << std::endl;

			float m_x[10], m_y[10], m_z[10];

			char buffer[OUTPUT_BUFFER_SIZE];
			osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);
		
			// Get the unlabeled markers
			unsigned int UnlabeledMarkerCount = MyClient.GetUnlabeledMarkerCount().MarkerCount;
			output_stream << "    Unlabeled Markers (" << UnlabeledMarkerCount << "):" << std::endl;


			for (unsigned int UnlabeledMarkerIndex = 0; UnlabeledMarkerIndex < 10; ++UnlabeledMarkerIndex)
			{
				// Get the global marker translation
				Output_GetUnlabeledMarkerGlobalTranslation _Output_GetUnlabeledMarkerGlobalTranslation =
				MyClient.GetUnlabeledMarkerGlobalTranslation(UnlabeledMarkerIndex);

				//output_stream << "      Marker #" << UnlabeledMarkerIndex << ": ("
				//<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[0] << ", "
				//<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[1] << ", "
				//<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[2] << ")" << std::endl;

				m_x[UnlabeledMarkerIndex] = (osc::int32)_Output_GetUnlabeledMarkerGlobalTranslation.Translation[0];
				m_y[UnlabeledMarkerIndex] = (osc::int32)_Output_GetUnlabeledMarkerGlobalTranslation.Translation[1];
				m_z[UnlabeledMarkerIndex] = (osc::int32)_Output_GetUnlabeledMarkerGlobalTranslation.Translation[2];
			}
			p << osc::BeginBundleImmediate
				<< osc::BeginMessage("/3d/marker")
				<< m_x[0] << m_y[0] << m_z[0] << osc::EndMessage;
			p << osc::EndBundle;
			transmitSocket.Send(p.Data(), p.Size());
		}

		if (EnableMultiCast)
		{
			MyClient.StopTransmittingMulticast();
		}
		//		MyClient.DisableSegmentData();
		MyClient.DisableMarkerData();
		/*	MyClient.DisableUnlabeledMarkerData();
		MyClient.DisableDeviceData();
		if (bReadCentroids)
		{
		MyClient.DisableCentroidData();
		}*/

		// Disconnect and dispose
		int t = clock();
		std::cout << " Disconnecting..." << std::endl;
		MyClient.Disconnect();
		int dt = clock() - t;
		double secs = (double)(dt) / (double)CLOCKS_PER_SEC;
		std::cout << " Disconnect time = " << secs << " secs" << std::endl;

	}
}
