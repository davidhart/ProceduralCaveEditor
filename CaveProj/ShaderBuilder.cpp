#include "ShaderBuilder.h"

#include <iostream>
#include <D3DX10.h>

//#define USE_PRECOMPILED_SHADERS

const std::string ShaderBuilder::compiledEffectExtension = ".fxo";
const std::string ShaderBuilder::effectSourceExtension = ".fx";

ID3D10Effect* ShaderBuilder::RequestEffect(const std::string& effectname, const std::string& profile, ID3D10Device* d3dDevice)
{

	std::string objectFileName = (effectname + compiledEffectExtension).c_str();
	std::string sourceFileName = (effectname + effectSourceExtension).c_str();

	HANDLE objectFile = CreateFile(objectFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HANDLE sourceFile = CreateFile(sourceFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	FILETIME objectFileTime;
	FILETIME sourceFileTime;

	if (objectFile != INVALID_HANDLE_VALUE)
	{
		GetFileTime(objectFile, NULL, NULL, &objectFileTime);
		CloseHandle(objectFile);
	}

	if (sourceFile != INVALID_HANDLE_VALUE)
	{
		GetFileTime(sourceFile, NULL, NULL, &sourceFileTime);
		CloseHandle(sourceFile);
	}

#ifndef USE_PRECOMPILED_SHADERS

	if (sourceFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "Effect \"" << effectname << "\" not found" << std::endl;
		return NULL;
	}

	if (objectFile != INVALID_HANDLE_VALUE && sourceFile != INVALID_HANDLE_VALUE && CompareFileTime(&sourceFileTime, &objectFileTime) <= 0)
	{

#endif
		objectFile = CreateFile(objectFileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (objectFile == INVALID_HANDLE_VALUE)
		{
			std::cout << "Could not read precompiled effect" << std::endl;
		}
		else
		{
			DWORD fileSize = GetFileSize(objectFile, NULL);
			DWORD bytesRead;
			unsigned char* buffer = new unsigned char[fileSize];
			ReadFile(objectFile, buffer, fileSize, &bytesRead, NULL);
			CloseHandle(objectFile);
			objectFile = INVALID_HANDLE_VALUE;

			ID3D10Effect* effect;
			ID3D10Blob* error;
			if (SUCCEEDED(D3DX10CreateEffectFromMemory(buffer,
									fileSize,
									sourceFileName.c_str(),
									NULL,
									NULL,
									profile.c_str(),
									0,
									0,
									d3dDevice,
									NULL,
									NULL,
									&effect,
									&error,
									NULL)))
			{
				delete [] buffer;
				std::cout << "Successfully created effect \"" << effectname << "\"" << std::endl;
				return effect;
			}

			if (error != NULL)
				error->Release();

			delete [] buffer;
		}

#ifndef USE_PRECOMPILED_SHADERS
	}

	if ((sourceFile != INVALID_HANDLE_VALUE))
	{

		if (objectFile != INVALID_HANDLE_VALUE && CompareFileTime(&sourceFileTime, &objectFileTime) > 0)
		{
			std::cout << "Precompiled effect \"" << effectname << "\" is out of date, building from source" << std::endl;
		}
		else
		{
			std::cout << "Couldn't load precompiled effect \"" << effectname << "\", building from source" << std::endl;
		}

		ID3D10Effect* effect;
		ID3D10Blob* error;
		ID3D10Blob* compiledshader;

		if (FAILED(D3DX10CompileFromFile(sourceFileName.c_str(), 
										 0,
										 0,
										 NULL,
										 profile.c_str(),
										 0,
										 0,
										 NULL,
										 &compiledshader,
										 &error,
										 0)))
		{
			std::cout << "Error compiling shader : " << std::endl;
			std::cout << (char*)error->GetBufferPointer() << std::endl;
			error->Release();
			return NULL;
		}

		objectFile = CreateFile(objectFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		BOOL saved = FALSE;

		if (objectFile != INVALID_HANDLE_VALUE)
		{
			DWORD numBytesWritten;
			saved = WriteFile(objectFile, compiledshader->GetBufferPointer(), compiledshader->GetBufferSize(), &numBytesWritten, NULL);

			CloseHandle(objectFile);
		}

		if (FAILED(D3DX10CreateEffectFromMemory(compiledshader->GetBufferPointer(),
									compiledshader->GetBufferSize(),
									sourceFileName.c_str(),
									NULL,
									NULL,
									profile.c_str(),
									0,
									0,
									d3dDevice,
									NULL,
									NULL,
									&effect,
									&error,
									NULL)))
		{
			std::cout << "Error creating effect : " << std::endl;
			std::cout << (char*)error->GetBufferPointer() << std::endl;
			error->Release();
			compiledshader->Release();
			return NULL;
		}

		compiledshader->Release();

		if (!saved)
		{
			std::cout << "Successfully created effect \"" << effectname << "\", but could not save object file" << std::endl;
		}
		else
		{
			std::cout << "Successfully created effect \"" << effectname << "\"" << std::endl;
		}

		return effect;
	}

#endif
	return NULL;
}
