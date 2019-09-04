#pragma once


struct ShaderSet
{
	
	Entity vertex;

	Entity tessellationControl;

	Entity tessellationEvaluation;

	Entity geometry;

	Entity fragment;
	
};

enum class Format {
	
	RGBA,
	Unknown
	
};