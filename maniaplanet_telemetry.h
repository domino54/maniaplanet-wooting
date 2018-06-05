/**
 * ManiaPlanet telemetry namespace.
 */

#ifndef _MANIAPLANET_TELEMETRY_H
#define _MANIAPLANET_TELEMETRY_H

#pragma once

namespace NManiaPlanet {
	enum {
		ECurVersion = 2,
	};
	
	typedef unsigned int uint;
	typedef unsigned int Bool;
	
	struct Vec3 {
		float X, Y, Z;
	};
	
	struct Quat {
		float W, X, Y, Z;
	};
	
	struct STelemetry {
		struct SHeader {
			char	Magic[32];	// == "ManiaPlanet_Telemetry"
			uint	Version;
			uint	Size;		// == sizeof(STelemetry)
		};
		
		enum EGameState {
			EState_Starting = 0,
			EState_Menus,
			EState_Running,
			EState_Paused,
		};
		
		enum ERaceState {
			ERaceState_BeforeState = 0,
			ERaceState_Running,
			ERaceState_Finished,
		};	
		
		struct SGameState {
			EGameState  State;
			char		PlayerModel[64];	// Player model name (eg. "CanyonCar")
			char		MapUID[64];
			char		MapName[256];		// Map name (without formatting)
			char		__future__[128];
		};
		
		struct SRaceState {
			ERaceState	State;
			uint		Time;
			uint		NbRespawns;
			uint		NbCheckpoints;
			uint		CheckpointTimes[125];
			char		__future__[32];
		};
		
		struct SObjectState {
			uint	Timestamp;
			uint	DiscontinuityCount;		// the number changes everytime the object is moved not continuously (== teleported).
			Quat	Rotation;
			Vec3	Position;	// +X is "left", +Y is "up", +Z is "front"
			Vec3	Velocity;	// World velocity
			uint	LatestStableGroundContactTime;
			char	__future__[32];
		};
		
		struct SVehicleState {
			uint	Timestamp;
			
			float	InputSteering;
			float	InputGasPedal;
			Bool	InputIsBraking;
			Bool	InputIsHorn;
			
			float	EngineRPM;			// 0 - 11000 for StadiumCar, 1500 - 10000 for other cars
			int		EngineCurGear;
			float	EngineTurboRatio;	// 1 -> turbo full (starting), 0 -> turbo empty (finished)
			Bool	EngineFreeWheeling;
			
			Bool	WheelsIsGroundContact[4];
			Bool	WheelsIsSliping[4];
			float	WheelsDamperLength[4];
			float	WheelsDamperRangeMin;
			float	WheelsDamperRangeMax;
			
			float	RumbleIntensity;
			
			uint	Speed;			// Unsigned KM/H
			Bool	IsInWater;
			Bool	IsSparkling;
			Bool	IsLightTrails;
			Bool	IsLightsOn;
			Bool	IsFlying;		// Long time since touching ground.
			
			char	__future__[32];
		};
		
		// VR chair
		struct SDeviceState {
			Vec3	Euler;				// Pitch, roll, yaw
			float	CenteredYaw;		// yaw accumulated + recentered to apply onto the device
			float	CenteredAltitude;	// Altitude accumulated + recentered
			char	__future__[32];
		};
		
		SHeader			Header;
		uint			UpdateNumber;
		SGameState		Game;
		SRaceState		Race;
		SObjectState	Object;
		SVehicleState	Vehicle;
		SDeviceState	Device;
	};
	
	static Vec3 RotateInv(Quat r, Vec3 v) {
		float t2	= -r.W * r.X;
		float t3	= -r.W * r.Y;
		float t4	= -r.W * r.Z;
		float t5	= -r.X * r.X;
		float t6	=  r.X * r.Y;
		float t7	=  r.X * r.Z;
		float t8	= -r.Y * r.Y;
		float t9	=  r.Y * r.Z;
		float t10	= -r.Z * r.Z;
		
		Vec3 result;
		result.X = 2 * ((t8 + t10) * v.X + (t6 -  t4) * v.Y + (t3 + t7) * v.Z ) + v.X;
		result.Y = 2 * ((t4 +  t6) * v.X + (t5 + t10) * v.Y + (t9 - t2) * v.Z ) + v.Y;
		result.Z = 2 * ((t7 -  t3) * v.X + (t2 +  t9) * v.Y + (t5 + t8) * v.Z ) + v.Z;
		return result;
	}
}

#endif // _MANIAPLANET_TELEMETRY_H
