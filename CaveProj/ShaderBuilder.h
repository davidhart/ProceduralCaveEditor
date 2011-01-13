#pragma once
#ifndef _SHADERBUILDER_H_
#define _SHADERBUILDER_H_

#include <D3D10.h>
#include <string>

class ShaderBuilder
{
private:

	static const std::string effectSourceExtension;
	static const std::string compiledEffectExtension;

public:

	static ID3D10Effect* RequestEffect(const std::string& effectname, const std::string& profile, ID3D10Device* d3dDevice);
};

#endif