#pragma once
#include "ofMain.h"

#define GRAPH_LENGTH 600

class Graph
{
public:
	int newIndex = 0;
	float overflowingQueue[GRAPH_LENGTH] = {0};
	float x, y, width, height, compressRatio;

	Graph(float _x, float _y, float _width, float _height) {
		x = _x;
		y = _y;
		width = _width;
		height = _height;
	}

	void insert(float data) {
		if (newIndex + 1 == GRAPH_LENGTH)
			newIndex = 0;
		overflowingQueue[newIndex] = data;
		newIndex++;
	}

	template<typename F>
	void run(F &lambda) {
		int index = newIndex;
		while (index < GRAPH_LENGTH) {
			lambda(overflowingQueue[index]);
			index++;
		}
		index = 0;
		while (index < newIndex) {
			lambda(overflowingQueue[index]);
			index++;
		}
	}

	void draw() {
		int increment = ofGetWidth() / GRAPH_LENGTH;
		int currX = 0;
		float prev = 0;

		run([&prev, &currX, increment, this](float data) {
			ofDrawLine(x + currX, y - prev, x + currX + increment, y - data);
			prev = data;
			currX += increment;
		});

		std::ostringstream text;
		text << prev;
		std::string txt(text.str());
		ofDrawBitmapString(txt, x + currX + increment, y - prev);
	}
	
};

