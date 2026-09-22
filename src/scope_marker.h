#pragma once
#include <string>
#include <vector>
#include <raylib.h>
#include <map>
#include <format>

#define SCOPE_MARKER(name, category) auto __##name = ScopeMarker(#name, #category)

struct MarkerData {
	std::string name;
	std::string category;
	double duration;
};

class ScopeMarker {
private:
	static std::vector<MarkerData> markers;
	static std::vector<MarkerData> averageMarkers;

	MarkerData data;

	long long frequency;
	long long startTime;

public:
	ScopeMarker(std::string name, std::string category = "");
	~ScopeMarker();

	static void BeginFrame();
	static void DrawMarkers();
};