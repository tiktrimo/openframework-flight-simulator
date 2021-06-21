#pragma once
#include "ofMain.h"

#define GRAPH_LENGTH 550

class Graph
{
public:
	//data
	float circularArr[GRAPH_LENGTH] = { 0 };
	// data's startIndex
	int start = 0;
	// title of graph
	char graphName[100];
	// x, y: coordination of top-left point, hiddenMul : graphical scale value
	float x, y, width, height, hiddenMul = 1;

	Graph(float _x, float _y, float _width, float _height) {
		x = _x;
		y = _y;
		width = _width;
		height = _height;
	}

	// insert new data. 
	void insert(float data) {
		if (start == GRAPH_LENGTH )
			start = 0;
		circularArr[start] = data;
		start++;
	}

	// get lambda function. execute lambda function for every data stored in circularArr
	template<typename F>
	void run(F &lambda) {
		int index = start;
		while (index < GRAPH_LENGTH) {
			lambda(circularArr[index]);
			index++;
		}
		index = 0;
		while (index < start) {
			lambda(circularArr[index]);
			index++;
		}
	}

	// draw graph.
	void draw() {
		int increment = 2;
		int currX = 0;
		float prev = circularArr[start] * hiddenMul;

		// draw line
		run([&prev, &currX, increment, this](float data) {
			ofDrawLine(x + currX, y - prev, x + currX + increment, y - data * hiddenMul);
			prev = data * hiddenMul;
			currX += increment;
		});

		// draw value and graph's title at end of the line
		std::ostringstream text;
		text << prev / hiddenMul;
		std::string txt(text.str());
		ofDrawBitmapString(txt, x + currX + increment, y - prev);
		ofDrawBitmapString(graphName, x + currX + increment, y - prev - 12);
	}
	
};

