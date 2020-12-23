#pragma once
using namespace System;

namespace GreyCorbel
{
	public enum class ResultType
	{
		OK,
		Error
	};

	public ref class CloneResult
	{
	public:
		property String^ SourcePrincipal;
		property String^ TargetPrincipal;
		property ResultType Result;
		property Exception^ ErrorDetail;
	};
}

