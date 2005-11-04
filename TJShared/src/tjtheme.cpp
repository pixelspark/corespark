#include "../include/tjshared.h"
using Gdiplus::Color;

ThemeSubscription _subscription_theme(GC::Hold(new Theme()));
ThemeSubscription _subscription_bright_theme(GC::Hold(new BrightTheme()));

Theme::Theme() {
}

Theme::~Theme() {
}

std::wstring Theme::GetName() { return std::wstring(L"Donker (Standaard)"); }

Color Theme::GetBackgroundColor() {
	return Color(0,0,0);
}

Color Theme::GetTimeBackgroundColor() {
	return Color(20,20,20);
}

Color Theme::GetPropertyBackgroundColor() {
	return Color(0,0,0);
}

Color Theme::GetLineColor() {
	return Color(50,50,50);
}

Color Theme::GetTextColor() {
	return Color(255,255,255);
}

Color Theme::GetActiveTrackColor() {
	return Color(25,25,25);
}

Color Theme::GetActiveStartColor() {
	return Color(110,110,110);
}

Color Theme::GetActiveEndColor() {
	return Color(50,50,50);
}

Color Theme::GetSplitterStartColor() {
	return Color(100,100,100);
}

Color Theme::GetSplitterEndColor() {
	return Color(20,20,20);
}

Color Theme::GetCurrentPositionColor() {
	return Color(255,255,255);
}

Color Theme::GetTrackDescriptionTextColor() {
	return Color(120,120,120);
}


Color Theme::GetFaderColor() {
	return Color(255,255,255);
}

// Bright theme

BrightTheme::BrightTheme() {
}

BrightTheme::~BrightTheme() {
}

Color BrightTheme::GetBackgroundColor() {
	return Color(255,255,255);
}

Color BrightTheme::GetTimeBackgroundColor() {
	return Color(236,233,216);
}

Color BrightTheme::GetPropertyBackgroundColor() {
	return Color(236,233,216);
}

Color BrightTheme::GetLineColor() {
	return Color(136, 133, 116);
}

Color BrightTheme::GetTextColor() {
	return Color(0,0,0);
}

Color BrightTheme::GetActiveTrackColor() {
	return Color(206,203,186);
}

Color BrightTheme::GetActiveStartColor() {
	return Color(206,203,186);
}

Color BrightTheme::GetActiveEndColor() {
	return Color(206,203,186);
}

Color BrightTheme::GetSplitterStartColor() {
	return Color(100,100,100);
}

Color BrightTheme::GetSplitterEndColor() {
	return Color(133,133,133);
}

Color BrightTheme::GetCurrentPositionColor() {
	return Color(0,0,0);
}

Color BrightTheme::GetTrackDescriptionTextColor() {
	return Color(120,120,120);
}

Color BrightTheme::GetFaderColor() {
	return Color(0,0,0);
}

std::wstring BrightTheme::GetName() { return std::wstring(L"Licht"); }