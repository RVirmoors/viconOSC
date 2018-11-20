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

			for (unsigned int SubjectIndex = 0; SubjectIndex < SubjectCount; ++SubjectIndex)
			{
		//		output_stream << "  Subject #" << SubjectIndex << std::endl;

				// Get the subject name
				std::string SubjectName = MyClient.GetSubjectName(SubjectIndex).SubjectName;
		//		output_stream << "    Name: " << SubjectName << std::endl;

				// Get the root segment
		//		std::string RootSegment = MyClient.GetSubjectRootSegmentName(SubjectName).SegmentName;
		//		output_stream << "    Root Segment: " << RootSegment << std::endl;

				// Count the number of segments
		//		unsigned int SegmentCount = MyClient.GetSegmentCount(SubjectName).SegmentCount;
		//		output_stream << "    Segments (" << SegmentCount << "):" << std::endl;
		//		for (unsigned int SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		//		{
//					output_stream << "      Segment #" << SegmentIndex << std::endl;

					// Get the segment name
	//				std::string SegmentName = MyClient.GetSegmentName(SubjectName, SegmentIndex).SegmentName;
//					output_stream << "        Name: " << SegmentName << std::endl;

					// Get the segment parent
	//				std::string SegmentParentName = MyClient.GetSegmentParentName(SubjectName, SegmentName).SegmentName;
//					output_stream << "        Parent: " << SegmentParentName << std::endl;

					// Get the segment's children
		//			unsigned int ChildCount = MyClient.GetSegmentChildCount(SubjectName, SegmentName).SegmentCount;
	//				output_stream << "     Children (" << ChildCount << "):" << std::endl;
		//			for (unsigned int ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
		//			{
		//				std::string ChildName = MyClient.GetSegmentChildName(SubjectName, SegmentName, ChildIndex).SegmentName;
	//					output_stream << "       " << ChildName << std::endl;
		//			}

					/*
					// Get the static segment translation
					Output_GetSegmentStaticTranslation _Output_GetSegmentStaticTranslation =
						MyClient.GetSegmentStaticTranslation(SubjectName, SegmentName);
					output_stream << "        Static Translation: (" << _Output_GetSegmentStaticTranslation.Translation[0] << ", "
						<< _Output_GetSegmentStaticTranslation.Translation[1] << ", "
						<< _Output_GetSegmentStaticTranslation.Translation[2] << ")" << std::endl;

					// Get the static segment rotation in helical co-ordinates
					Output_GetSegmentStaticRotationHelical _Output_GetSegmentStaticRotationHelical =
						MyClient.GetSegmentStaticRotationHelical(SubjectName, SegmentName);
					output_stream << "        Static Rotation Helical: (" << _Output_GetSegmentStaticRotationHelical.Rotation[0] << ", "
						<< _Output_GetSegmentStaticRotationHelical.Rotation[1] << ", "
						<< _Output_GetSegmentStaticRotationHelical.Rotation[2] << ")" << std::endl;

					// Get the static segment rotation as a matrix
					Output_GetSegmentStaticRotationMatrix _Output_GetSegmentStaticRotationMatrix =
						MyClient.GetSegmentStaticRotationMatrix(SubjectName, SegmentName);
					output_stream << "        Static Rotation Matrix: (" << _Output_GetSegmentStaticRotationMatrix.Rotation[0] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[1] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[2] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[3] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[4] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[5] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[6] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[7] << ", "
						<< _Output_GetSegmentStaticRotationMatrix.Rotation[8] << ")" << std::endl;

					// Get the static segment rotation in quaternion co-ordinates
					Output_GetSegmentStaticRotationQuaternion _Output_GetSegmentStaticRotationQuaternion =
						MyClient.GetSegmentStaticRotationQuaternion(SubjectName, SegmentName);
					output_stream << "        Static Rotation Quaternion: (" << _Output_GetSegmentStaticRotationQuaternion.Rotation[0] << ", "
						<< _Output_GetSegmentStaticRotationQuaternion.Rotation[1] << ", "
						<< _Output_GetSegmentStaticRotationQuaternion.Rotation[2] << ", "
						<< _Output_GetSegmentStaticRotationQuaternion.Rotation[3] << ")" << std::endl;

					// Get the static segment rotation in EulerXYZ co-ordinates
					Output_GetSegmentStaticRotationEulerXYZ _Output_GetSegmentStaticRotationEulerXYZ =
						MyClient.GetSegmentStaticRotationEulerXYZ(SubjectName, SegmentName);
					output_stream << "        Static Rotation EulerXYZ: (" << _Output_GetSegmentStaticRotationEulerXYZ.Rotation[0] << ", "
						<< _Output_GetSegmentStaticRotationEulerXYZ.Rotation[1] << ", "
						<< _Output_GetSegmentStaticRotationEulerXYZ.Rotation[2] << ")" << std::endl;

					// Get the global segment translation
					Output_GetSegmentGlobalTranslation _Output_GetSegmentGlobalTranslation =
						MyClient.GetSegmentGlobalTranslation(SubjectName, SegmentName);
					output_stream << "        Global Translation: (" << _Output_GetSegmentGlobalTranslation.Translation[0] << ", "
						<< _Output_GetSegmentGlobalTranslation.Translation[1] << ", "
						<< _Output_GetSegmentGlobalTranslation.Translation[2] << ") "
						<< Adapt(_Output_GetSegmentGlobalTranslation.Occluded) << std::endl;

					// Get the global segment rotation in helical co-ordinates
					Output_GetSegmentGlobalRotationHelical _Output_GetSegmentGlobalRotationHelical =
						MyClient.GetSegmentGlobalRotationHelical(SubjectName, SegmentName);
					output_stream << "        Global Rotation Helical: (" << _Output_GetSegmentGlobalRotationHelical.Rotation[0] << ", "
						<< _Output_GetSegmentGlobalRotationHelical.Rotation[1] << ", "
						<< _Output_GetSegmentGlobalRotationHelical.Rotation[2] << ") "
						<< Adapt(_Output_GetSegmentGlobalRotationHelical.Occluded) << std::endl;

					// Get the global segment rotation as a matrix
					Output_GetSegmentGlobalRotationMatrix _Output_GetSegmentGlobalRotationMatrix =
						MyClient.GetSegmentGlobalRotationMatrix(SubjectName, SegmentName);
					output_stream << "        Global Rotation Matrix: (" << _Output_GetSegmentGlobalRotationMatrix.Rotation[0] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[1] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[2] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[3] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[4] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[5] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[6] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[7] << ", "
						<< _Output_GetSegmentGlobalRotationMatrix.Rotation[8] << ") "
						<< Adapt(_Output_GetSegmentGlobalRotationMatrix.Occluded) << std::endl;

					// Get the global segment rotation in quaternion co-ordinates
					Output_GetSegmentGlobalRotationQuaternion _Output_GetSegmentGlobalRotationQuaternion =
						MyClient.GetSegmentGlobalRotationQuaternion(SubjectName, SegmentName);
					output_stream << "        Global Rotation Quaternion: (" << _Output_GetSegmentGlobalRotationQuaternion.Rotation[0] << ", "
						<< _Output_GetSegmentGlobalRotationQuaternion.Rotation[1] << ", "
						<< _Output_GetSegmentGlobalRotationQuaternion.Rotation[2] << ", "
						<< _Output_GetSegmentGlobalRotationQuaternion.Rotation[3] << ") "
						<< Adapt(_Output_GetSegmentGlobalRotationQuaternion.Occluded) << std::endl;

					// Get the global segment rotation in EulerXYZ co-ordinates
					Output_GetSegmentGlobalRotationEulerXYZ _Output_GetSegmentGlobalRotationEulerXYZ =
						MyClient.GetSegmentGlobalRotationEulerXYZ(SubjectName, SegmentName);
					output_stream << "        Global Rotation EulerXYZ: (" << _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[0] << ", "
						<< _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[1] << ", "
						<< _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[2] << ") "
						<< Adapt(_Output_GetSegmentGlobalRotationEulerXYZ.Occluded) << std::endl;

					// Get the local segment translation
					Output_GetSegmentLocalTranslation _Output_GetSegmentLocalTranslation =
						MyClient.GetSegmentLocalTranslation(SubjectName, SegmentName);
					output_stream << "        Local Translation: (" << _Output_GetSegmentLocalTranslation.Translation[0] << ", "
						<< _Output_GetSegmentLocalTranslation.Translation[1] << ", "
						<< _Output_GetSegmentLocalTranslation.Translation[2] << ") "
						<< Adapt(_Output_GetSegmentLocalTranslation.Occluded) << std::endl;

					// Get the local segment rotation in helical co-ordinates
					Output_GetSegmentLocalRotationHelical _Output_GetSegmentLocalRotationHelical =
						MyClient.GetSegmentLocalRotationHelical(SubjectName, SegmentName);
					output_stream << "        Local Rotation Helical: (" << _Output_GetSegmentLocalRotationHelical.Rotation[0] << ", "
						<< _Output_GetSegmentLocalRotationHelical.Rotation[1] << ", "
						<< _Output_GetSegmentLocalRotationHelical.Rotation[2] << ") "
						<< Adapt(_Output_GetSegmentLocalRotationHelical.Occluded) << std::endl;

					// Get the local segment rotation as a matrix
					Output_GetSegmentLocalRotationMatrix _Output_GetSegmentLocalRotationMatrix =
						MyClient.GetSegmentLocalRotationMatrix(SubjectName, SegmentName);
					output_stream << "        Local Rotation Matrix: (" << _Output_GetSegmentLocalRotationMatrix.Rotation[0] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[1] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[2] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[3] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[4] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[5] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[6] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[7] << ", "
						<< _Output_GetSegmentLocalRotationMatrix.Rotation[8] << ") "
						<< Adapt(_Output_GetSegmentLocalRotationMatrix.Occluded) << std::endl;

					// Get the local segment rotation in quaternion co-ordinates
					Output_GetSegmentLocalRotationQuaternion _Output_GetSegmentLocalRotationQuaternion =
						MyClient.GetSegmentLocalRotationQuaternion(SubjectName, SegmentName);
					output_stream << "        Local Rotation Quaternion: (" << _Output_GetSegmentLocalRotationQuaternion.Rotation[0] << ", "
						<< _Output_GetSegmentLocalRotationQuaternion.Rotation[1] << ", "
						<< _Output_GetSegmentLocalRotationQuaternion.Rotation[2] << ", "
						<< _Output_GetSegmentLocalRotationQuaternion.Rotation[3] << ") "
						<< Adapt(_Output_GetSegmentLocalRotationQuaternion.Occluded) << std::endl;

					// Get the local segment rotation in EulerXYZ co-ordinates
					Output_GetSegmentLocalRotationEulerXYZ _Output_GetSegmentLocalRotationEulerXYZ =
						MyClient.GetSegmentLocalRotationEulerXYZ(SubjectName, SegmentName);
					output_stream << "        Local Rotation EulerXYZ: (" << _Output_GetSegmentLocalRotationEulerXYZ.Rotation[0] << ", "
						<< _Output_GetSegmentLocalRotationEulerXYZ.Rotation[1] << ", "
						<< _Output_GetSegmentLocalRotationEulerXYZ.Rotation[2] << ") "
						<< Adapt(_Output_GetSegmentLocalRotationEulerXYZ.Occluded) << std::endl;
						*/
				//	}


				// Count the number of markers
				unsigned int MarkerCount = MyClient.GetMarkerCount(SubjectName).MarkerCount;
			//	output_stream << "    Markers (" << MarkerCount << "):" << std::endl;
				for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
				{
					// Get the marker name
					std::string MarkerName = MyClient.GetMarkerName(SubjectName, MarkerIndex).MarkerName;

					// Get the marker parent
			//		std::string MarkerParentName = MyClient.GetMarkerParentName(SubjectName, MarkerName).SegmentName;

					// Get the global marker translation
					Output_GetMarkerGlobalTranslation _Output_GetMarkerGlobalTranslation =
						MyClient.GetMarkerGlobalTranslation(SubjectName, MarkerName);

				/*	output_stream << "      Marker #" << MarkerIndex << ": "
						<< MarkerName << " ("
						<< _Output_GetMarkerGlobalTranslation.Translation[0] << ", "
						<< _Output_GetMarkerGlobalTranslation.Translation[1] << ", "
						<< _Output_GetMarkerGlobalTranslation.Translation[2] << ") "
						<< Adapt(_Output_GetMarkerGlobalTranslation.Occluded) << std::endl;
					*/ 
						char buffer[OUTPUT_BUFFER_SIZE];
						osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

						/*
						p << osc::BeginBundleImmediate
							<< osc::BeginMessage("/markers")
							<< (osc::int32)SubjectIndex << (osc::int32)MarkerIndex << m_x << m_z << m_y 
							<< (MarkerName.c_str()) << osc::EndMessage
							<< osc::EndBundle; */

						transmitSocket.Send(p.Data(), p.Size()); 

						m_x[MarkerIndex] = (osc::int32)_Output_GetMarkerGlobalTranslation.Translation[0];
						m_y[MarkerIndex] = (osc::int32)_Output_GetMarkerGlobalTranslation.Translation[1];
						m_z[MarkerIndex] = (osc::int32)_Output_GetMarkerGlobalTranslation.Translation[2];

						//m_x[MarkerIndex] /= 3000.;// = (m_x + 0) / 3000.;
						//m_y[MarkerIndex] /= 3000.;// = (m_y - 0) / 3000.;
						//m_z[MarkerIndex] /= 3000.;// = (m_z + 0) / 3000.;
									
				}
			}



			char buffer[OUTPUT_BUFFER_SIZE];
			osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

			// 0, 2 = head
			// 1, 3 = elbow
			// 4, 5 = hand

			if (!m_x[0]) m_x[0] = m_x[2];
			if (!m_x[2]) m_x[2] = m_x[0];
			if (!m_y[0]) m_y[0] = m_y[2];
			if (!m_y[2]) m_y[2] = m_y[0];
			if (!m_z[0]) m_z[0] = m_z[2];
			if (!m_z[2]) m_z[2] = m_z[0];

		/*	if (!m_x[1]) m_x[1] = m_x[2];
			if (!m_x[2]) m_x[2] = m_x[1];
			if (!m_y[1]) m_y[1] = m_y[2];
			if (!m_y[2]) m_y[2] = m_y[1];
			if (!m_z[1]) m_z[1] = m_z[2];
			if (!m_z[2]) m_z[2] = m_z[1];*/

			if (!m_x[5]) m_x[5] = m_x[4];
			if (!m_x[4]) m_x[4] = m_x[5];
			if (!m_y[5]) m_y[5] = m_y[4];
			if (!m_y[4]) m_y[4] = m_y[5];
			if (!m_z[5]) m_z[5] = m_z[4];
			if (!m_z[4]) m_z[4] = m_z[5];

			if (m_x[0] && m_x[5] ) {
				p << osc::BeginBundleImmediate
					<< osc::BeginMessage("/markers/cap")
					<< (m_x[0] + m_x[2]) / 2 << (m_y[0] + m_y[2]) / 2 << (m_z[0] + m_z[2]) / 2 << osc::EndMessage;

				//p << osc::BeginMessage("/markers/dreapta")
				//	<< (m_x[1] + m_x[2]) /2 << (m_y[1] + m_y[2]) / 2 << (m_z[1] + m_z[2]) / 2 << osc::EndMessage;

				p << osc::BeginMessage("/markers/stanga")
					<< (m_x[4] + m_x[5]) / 2 << (m_y[4] + m_y[5]) / 2 << (m_z[4] + m_z[5]) / 2 << osc::EndMessage;

				float dist = pow(m_x[5] - m_x[4], 2) + pow(m_y[5] - m_y[4], 2) + pow(m_z[5] - m_z[4], 2);

				p << osc::BeginMessage("/rec/dist")	<< dist
					<< osc::EndMessage;

				if (dist < 3000 && dist > 1)
					p << osc::BeginMessage("/rec/1") << osc::EndMessage;
				else
					if (dist >= 3000)
						p << osc::BeginMessage("/rec/0") << osc::EndMessage;


				p << osc::EndBundle;
				transmitSocket.Send(p.Data(), p.Size());
			}


//			Sleep(1500);
			/*
			// Get the unlabeled markers
			unsigned int UnlabeledMarkerCount = MyClient.GetUnlabeledMarkerCount().MarkerCount;
			output_stream << "    Unlabeled Markers (" << UnlabeledMarkerCount << "):" << std::endl;


			for (unsigned int UnlabeledMarkerIndex = 0; UnlabeledMarkerIndex < 10; ++UnlabeledMarkerIndex)
			{
				// Get the global marker translation
				Output_GetUnlabeledMarkerGlobalTranslation _Output_GetUnlabeledMarkerGlobalTranslation =
					MyClient.GetUnlabeledMarkerGlobalTranslation(UnlabeledMarkerIndex);

				output_stream << "      Marker #" << UnlabeledMarkerIndex << ": ("
					<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[0] << ", "
					<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[1] << ", "
					<< _Output_GetUnlabeledMarkerGlobalTranslation.Translation[2] << ")" << std::endl;
			}*/
			/*
			p << osc::BeginBundleImmediate
				<< osc::BeginMessage("/latency/")
				<< (float)MyClient.GetLatencyTotal().Total << osc::EndMessage;

			p << osc::EndBundle;
			transmitSocket.Send(p.Data(), p.Size());*/

			/*
			// Count the number of devices
			unsigned int DeviceCount = MyClient.GetDeviceCount().DeviceCount;
			output_stream << "  Devices (" << DeviceCount << "):" << std::endl;
			for (unsigned int DeviceIndex = 0; DeviceIndex < DeviceCount; ++DeviceIndex)
			{
				output_stream << "    Device #" << DeviceIndex << ":" << std::endl;

				// Get the device name and type
				Output_GetDeviceName _Output_GetDeviceName = MyClient.GetDeviceName(DeviceIndex);
				output_stream << "      Name: " << _Output_GetDeviceName.DeviceName << std::endl;
				output_stream << "      Type: " << Adapt(_Output_GetDeviceName.DeviceType) << std::endl;

				// Count the number of device outputs
				unsigned int DeviceOutputCount = MyClient.GetDeviceOutputCount(_Output_GetDeviceName.DeviceName).DeviceOutputCount;
				output_stream << "      Device Outputs (" << DeviceOutputCount << "):" << std::endl;
				for (unsigned int DeviceOutputIndex = 0; DeviceOutputIndex < DeviceOutputCount; ++DeviceOutputIndex)
				{
					// Get the device output name and unit
					Output_GetDeviceOutputName _Output_GetDeviceOutputName =
						MyClient.GetDeviceOutputName(_Output_GetDeviceName.DeviceName, DeviceOutputIndex);

					unsigned int DeviceOutputSubsamples =
						MyClient.GetDeviceOutputSubsamples(_Output_GetDeviceName.DeviceName,
							_Output_GetDeviceOutputName.DeviceOutputName).DeviceOutputSubsamples;

					output_stream << "      Device Output #" << DeviceOutputIndex << ":" << std::endl;
					output_stream << "      Samples (" << DeviceOutputSubsamples << "):" << std::endl;

					for (unsigned int DeviceOutputSubsample = 0; DeviceOutputSubsample < DeviceOutputSubsamples; ++DeviceOutputSubsample)
					{
						output_stream << "        Sample #" << DeviceOutputSubsample << ":" << std::endl;

						// Get the device output value
						Output_GetDeviceOutputValue _Output_GetDeviceOutputValue =
							MyClient.GetDeviceOutputValue(_Output_GetDeviceName.DeviceName,
								_Output_GetDeviceOutputName.DeviceOutputName,
								DeviceOutputSubsample);

						output_stream << "          '" << _Output_GetDeviceOutputName.DeviceOutputName << "' "
							<< _Output_GetDeviceOutputValue.Value << " "
							<< Adapt(_Output_GetDeviceOutputName.DeviceOutputUnit) << " "
							<< Adapt(_Output_GetDeviceOutputValue.Occluded) << std::endl;
					}
				}
			}*/
			/*
			// Output the force plate information.
			unsigned int ForcePlateCount = MyClient.GetForcePlateCount().ForcePlateCount;
			output_stream << "  Force Plates: (" << ForcePlateCount << ")" << std::endl;

			for (unsigned int ForcePlateIndex = 0; ForcePlateIndex < ForcePlateCount; ++ForcePlateIndex)
			{
				output_stream << "    Force Plate #" << ForcePlateIndex << ":" << std::endl;

				unsigned int ForcePlateSubsamples = MyClient.GetForcePlateSubsamples(ForcePlateIndex).ForcePlateSubsamples;

				output_stream << "    Samples (" << ForcePlateSubsamples << "):" << std::endl;

				for (unsigned int ForcePlateSubsample = 0; ForcePlateSubsample < ForcePlateSubsamples; ++ForcePlateSubsample)
				{
					output_stream << "      Sample #" << ForcePlateSubsample << ":" << std::endl;

					Output_GetGlobalForceVector _Output_GetForceVector = MyClient.GetGlobalForceVector(ForcePlateIndex, ForcePlateSubsample);
					output_stream << "        Force (" << _Output_GetForceVector.ForceVector[0] << ", ";
					output_stream << _Output_GetForceVector.ForceVector[1] << ", ";
					output_stream << _Output_GetForceVector.ForceVector[2] << ")" << std::endl;

					Output_GetGlobalMomentVector _Output_GetMomentVector =
						MyClient.GetGlobalMomentVector(ForcePlateIndex, ForcePlateSubsample);
					output_stream << "        Moment (" << _Output_GetMomentVector.MomentVector[0] << ", ";
					output_stream << _Output_GetMomentVector.MomentVector[1] << ", ";
					output_stream << _Output_GetMomentVector.MomentVector[2] << ")" << std::endl;

					Output_GetGlobalCentreOfPressure _Output_GetCentreOfPressure =
						MyClient.GetGlobalCentreOfPressure(ForcePlateIndex, ForcePlateSubsample);
					output_stream << "        CoP (" << _Output_GetCentreOfPressure.CentreOfPressure[0] << ", ";
					output_stream << _Output_GetCentreOfPressure.CentreOfPressure[1] << ", ";
					output_stream << _Output_GetCentreOfPressure.CentreOfPressure[2] << ")" << std::endl;
				}
			}*/

			// Output eye tracker information.
			/*
			unsigned int EyeTrackerCount = MyClient.GetEyeTrackerCount().EyeTrackerCount;
			output_stream << "  Eye Trackers: (" << EyeTrackerCount << ")" << std::endl;

			for (unsigned int EyeTrackerIndex = 0; EyeTrackerIndex < EyeTrackerCount; ++EyeTrackerIndex)
			{
				output_stream << "    Eye Tracker #" << EyeTrackerIndex << ":" << std::endl;

				Output_GetEyeTrackerGlobalPosition _Output_GetEyeTrackerGlobalPosition = MyClient.GetEyeTrackerGlobalPosition(EyeTrackerIndex);

				output_stream << "      Position (" << _Output_GetEyeTrackerGlobalPosition.Position[0] << ", ";
				output_stream << _Output_GetEyeTrackerGlobalPosition.Position[1] << ", ";
				output_stream << _Output_GetEyeTrackerGlobalPosition.Position[2] << ") ";
				output_stream << Adapt(_Output_GetEyeTrackerGlobalPosition.Occluded) << std::endl;

				Output_GetEyeTrackerGlobalGazeVector _Output_GetEyeTrackerGlobalGazeVector = MyClient.GetEyeTrackerGlobalGazeVector(EyeTrackerIndex);

				output_stream << "      Gaze (" << _Output_GetEyeTrackerGlobalGazeVector.GazeVector[0] << ", ";
				output_stream << _Output_GetEyeTrackerGlobalGazeVector.GazeVector[1] << ", ";
				output_stream << _Output_GetEyeTrackerGlobalGazeVector.GazeVector[2] << ") ";
				output_stream << Adapt(_Output_GetEyeTrackerGlobalGazeVector.Occluded) << std::endl;
			}
			*/
			/*
			if (bReadCentroids)
			{
				unsigned int CameraCount = MyClient.GetCameraCount().CameraCount;
				output_stream << "Cameras(" << CameraCount << "):" << std::endl;

				for (unsigned int CameraIndex = 0; CameraIndex < CameraCount; ++CameraIndex)
				{
					output_stream << "  Camera #" << CameraIndex << ":" << std::endl;

					const std::string CameraName = MyClient.GetCameraName(CameraIndex).CameraName;
					output_stream << "    Name: " << CameraName << std::endl;

					unsigned int CentroidCount = MyClient.GetCentroidCount(CameraName).CentroidCount;
					output_stream << "    Centroids(" << CentroidCount << "):" << std::endl;

					for (unsigned int CentroidIndex = 0; CentroidIndex < CentroidCount; ++CentroidIndex)
					{
						output_stream << "      Centroid #" << CentroidIndex << ":" << std::endl;

						Output_GetCentroidPosition _Output_GetCentroidPosition = MyClient.GetCentroidPosition(CameraName, CentroidIndex);
						output_stream << "        Position: (" << _Output_GetCentroidPosition.CentroidPosition[0] << ", "
							<< _Output_GetCentroidPosition.CentroidPosition[1] << ")" << std::endl;
						output_stream << "        Radius: (" << _Output_GetCentroidPosition.Radius << ")" << std::endl;
						//output_stream << "        Accuracy: ("  << _Output_GetCentroidPosition.Accuracy << ")" << std::endl;
					}
				}
			}*/
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
