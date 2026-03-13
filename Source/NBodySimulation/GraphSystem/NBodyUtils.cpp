#include "NBodyUtils.h"
#include <fstream>
#include "Engine/Engine.h"

static std::string GetLogFilePath()
{
	static const FString FilePath1 = FPaths::ProjectSavedDir() + TEXT("Logs/GraphSystem_Debug.log");
	static const std::string FilePath = TCHAR_TO_UTF8(*FilePath1);
	return FilePath;
}

void ClearLogFile()
{
	std::ofstream LogFile(GetLogFilePath().c_str(), std::ios::trunc);
	if (LogFile.is_open())
	{
		LogFile.close();
		UE_LOG(LogTemp, Log, TEXT("GraphSystem log file cleared successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to clear GraphSystem log file at: %s"), *FString(GetLogFilePath().c_str()));
	}
}

void LogAlways(const FString& TextToWrite)
{
	// Open/close each time for simplicity and hot-reload safety
	// Performance note: For high-frequency logging, consider batching or using a persistent handle
	std::ofstream LogFile(GetLogFilePath().c_str(), std::ios::app);
	if (LogFile.is_open())
	{
		LogFile << TCHAR_TO_ANSI(*TextToWrite) << std::endl;
		LogFile.close();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to open GraphSystem log file for writing: %s"), *FString(GetLogFilePath().c_str()));
	}
}

void LogMessageInternal(const FString& StringToLog, bool LOG, int SeverityLevel, const FString& Prefix)
{
	if (!LOG || StringToLog.IsEmpty())
	{
		return;
	}
	
	FString LogMessage = Prefix + StringToLog;

	switch (SeverityLevel)
	{
	case 0:
		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
		break;
	case 1:
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
		break;
	case 2:
		UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
		break;
	}

	LogAlways(StringToLog);
}

FVector Jiggle(const FVector& Vec, float Magnitude)
{
	return Vec;
}

void eeeee()
{
}
