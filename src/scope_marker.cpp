#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOUSER
#define NOGDI
#include <windows.h>
#include <algorithm>

#include "scope_marker.h"

int ScopeMarker::currentDepth = 0;

std::vector<MarkerData> ScopeMarker::markers;
std::vector<MarkerData> ScopeMarker::averageMarkers;

ScopeMarker::ScopeMarker(std::string name, std::string category) {
	data.name = name;
	data.category = category;
	data.depth = currentDepth;
	currentDepth++;

	LARGE_INTEGER timerFrequency;
	LARGE_INTEGER timerStart;
	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);

	frequency = timerFrequency.QuadPart;
	startTime = timerStart.QuadPart;
}

ScopeMarker::~ScopeMarker() {
	currentDepth--;

	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	data.duration = static_cast<double>(currentTime.QuadPart - startTime) * 1000.0 / frequency;

	markers.push_back(data);
}

void ScopeMarker::BeginFrame() {
	std::vector<MarkerData> frameAverages;
	for (const MarkerData& marker : markers) {
		auto existing = std::find_if(averageMarkers.begin(), averageMarkers.end(), [&](const MarkerData& averageMarker) {
			return averageMarker.name == marker.name && averageMarker.category == marker.category;
		});
		if (existing == averageMarkers.end())
			frameAverages.push_back(marker);
		else
			frameAverages.push_back({ marker.name, marker.category, marker.depth, (marker.duration + existing->duration) / 2 });
	}

	averageMarkers.swap(frameAverages);
	markers.clear();
}

void ScopeMarker::DrawMarkers() {
	int fontSize = 18;
	int y = 5 + fontSize;

	std::string previousCategory = "";
	for (const MarkerData& marker : averageMarkers) {
		if (previousCategory != marker.category) {
			DrawText(marker.category.c_str(), 5, y + 5, fontSize, BLACK);
			previousCategory = marker.category;
			y += fontSize + 5;
		}
		DrawText(("	" + marker.name).c_str(), 5 + marker.depth * 10, y, fontSize, BLACK);
		DrawText(std::format("{:.2f}ms", marker.duration).c_str(), 220, y, fontSize, BLACK);

		y += fontSize;
	}
}
