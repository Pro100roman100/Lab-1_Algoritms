#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOUSER
#define NOGDI
#include <windows.h>

#include "scope_marker.h"

std::vector<MarkerData> ScopeMarker::markers;
std::vector<MarkerData> ScopeMarker::averageMarkers;

ScopeMarker::ScopeMarker(std::string name, std::string category) {
	data.name = name;
	data.category = category;

	LARGE_INTEGER timerFrequency;
	LARGE_INTEGER timerStart;
	QueryPerformanceFrequency(&timerFrequency);
	QueryPerformanceCounter(&timerStart);

	frequency = timerFrequency.QuadPart;
	startTime = timerStart.QuadPart;
}

ScopeMarker::~ScopeMarker() {
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	data.duration = static_cast<double>(currentTime.QuadPart - startTime) * 1000.0 / frequency;

	markers.push_back(data);
}

void ScopeMarker::BeginFrame() {
	averageMarkers.resize(markers.size());

	for (int i = 0; i < markers.size(); i++) {
		if (markers[i].name == averageMarkers[i].name)
			averageMarkers[i].duration = (markers[i].duration + averageMarkers[i].duration) / 2;
		else
			averageMarkers[i] = markers[i];
	}

	markers.clear();
}

void ScopeMarker::DrawMarkers() {
	int y = 5;
	int fontSize = 18;

	std::string previousCategory = "";
	for (const MarkerData& marker : averageMarkers) {
		if (previousCategory != marker.category) {
			DrawText(marker.category.c_str(), 5, y + 5, fontSize, BLACK);
			previousCategory = marker.category;
			y += fontSize + 5;
		}
		DrawText(("	" + marker.name).c_str(), 5, y, fontSize, BLACK);
		DrawText(std::format("{:.2f}ms", marker.duration).c_str(), 180, y, fontSize, BLACK);

		y += fontSize;
	}
}
